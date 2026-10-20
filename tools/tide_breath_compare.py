#!/usr/bin/env python3
"""P0a dry listening pack and baseline proofs. Requires NumPy, no device I/O.

Compile tools/tide-breath-render.c against pristine source, current default,
and current -DTIDE_BREATH_EXPERIMENT=1 into build/p0a/{baseline,factory,candidate}-render.
Run from repository root: python tools/tide_breath_compare.py [OUTPUT_DIR].
Each take is a fresh process. All matching is one fixed gain per recording.
"""
import hashlib
import html
import json
from pathlib import Path
import subprocess
import sys
import wave
import numpy as np

out = Path(sys.argv[1] if len(sys.argv)>1 else 'build/p0a/listening')
out.mkdir(parents=True, exist_ok=True)
if any(out.iterdir()):
    raise SystemExit(f'Refusing to replace existing listening pack: {out}')
bins = {name: Path('build/p0a') / f'{name}-render' for name in ('baseline','factory','candidate')}
report = {'baseline_revision':'828ceaa', 'matching':'one fixed gain per take; RMS seconds 8..release; target 6000/32768',
          'notes':{'single':[50], 'chord':[50,57,65,72], 'high':[62,69,77,84]}, 'takes':{}, 'proofs':{}, 'cpu':{}}

def render(binary, path, sound, seconds, motion, gestures=0):
    p = subprocess.run([str(binary),str(path),sound,str(seconds),str(motion),str(gestures)],text=True,capture_output=True)
    if p.returncode: raise RuntimeError(p.stdout+p.stderr)
    result=json.loads(p.stdout)
    if sound=='all-tide':
        assert result.get('voices',result.get('max_voices'))==8, 'Eight-Tide stress must actually sound eight voices'
    return result

def read(path):
    with wave.open(str(path),'rb') as w:
        assert w.getnchannels()==2 and w.getsampwidth()==2 and w.getframerate()==44100
        return np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,2)

def write(path, samples):
    with wave.open(str(path),'wb') as w:
        w.setparams((2,2,44100,0,'NONE','not compressed'))
        w.writeframes(np.asarray(samples,dtype='<i2').tobytes())

def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()

def measurements(samples, seconds):
    mono=samples[:,0].astype(np.float64)
    held=mono[8*44100:seconds*44100]
    rms=float(np.sqrt(np.mean(held**2)))
    levels=[20*np.log10(max(1,np.sqrt(np.mean(mono[s*44100:(s+1)*44100]**2)))) for s in range(8,seconds)]
    # Windowed spectra across the sustain; report proportions, not a quality score.
    spectra=[]
    for second in np.linspace(8,seconds-1,24).astype(int):
        block=mono[second*44100:second*44100+32768]
        spectra.append(abs(np.fft.rfft(block*np.hanning(len(block))))**2)
    power=np.mean(spectra,axis=0); hz=np.fft.rfftfreq(32768,1/44100)
    total=float(power[(hz>=20)&(hz<20000)].sum())
    bands={f'{lo}-{hi}Hz':float(power[(hz>=lo)&(hz<hi)].sum()/total) for lo,hi in ((20,80),(80,300),(300,1500),(1500,8000))}
    return {'rms':rms,'one_second_level_range_db':float(max(levels)-min(levels)),
            'one_second_level_std_db':float(np.std(levels)), 'band_energy_fraction':bands}

cases=[('single',40,25,0),('chord',40,25,0),('high',40,25,0),
       ('single-long',300,25,0),('chord-long',300,25,0),
       ('single-still',40,0,0),('chord-still',40,0,0),
       ('chord-motion100',60,100,0),('chord-gestures',40,25,1)]
rows=[]
for name,seconds,motion,gestures in cases:
    sound=name.split('-')[0]
    paired=[]
    for label,binary in (('A',bins['baseline']),('B',bins['candidate'])):
        path=out/f'{name}-{label}-raw.wav'
        stats=render(binary,path,sound,seconds,motion,gestures)
        samples=read(path); measures=measurements(samples,seconds)
        gain=6000/measures['rms']
        scaled=np.rint(samples.astype(np.float64)*gain)
        assert abs(scaled).max()<32767, f'{name}: fixed matching clips'
        matched=out/f'{name}-{label}-matched.wav'; write(matched,scaled)
        actual=float(np.sqrt(np.mean(scaled[8*44100:seconds*44100]**2)))
        stats.update(measures,gain_db=20*np.log10(gain),matched_rms=actual,
                     matched_peak=int(abs(scaled).max()),raw_sha256=digest(path))
        report['takes'][f'{name}-{label}']=stats
        paired.append(scaled.astype('<i2'))
        print(f'{name} {label}: RMS {measures["rms"]:.1f}, gain {stats["gain_db"]:.2f} dB',flush=True)
    if motion==0:
        assert digest(out/f'{name}-A-raw.wav')==digest(out/f'{name}-B-raw.wav'), 'Motion0 differs'
        report['proofs'][name+'-exact-bypass']=True
    write(out/f'{name}-AB.wav',np.concatenate((paired[0],np.zeros((44100,2)),paired[1])))
    rows.append(f'<section><h2>{html.escape(name)} — Motion {motion}</h2><p>A = unchanged Tide; B = opt-in breathing. {seconds}s held + 12s release each; 1s gap.</p><audio controls preload="none" src="{name}-AB.wav"></audio><p><a href="{name}-A-matched.wav">A separately</a> · <a href="{name}-B-matched.wav">B separately</a></p></section>')

# Default build and excluded sounds must render identically to pristine code.
for sound in ('single','chord','high','mix','all-tide','warm','silk','scatter'):
    reference=out/f'proof-{sound}-baseline.wav'
    default=out/f'proof-{sound}-factory.wav'
    render(bins['baseline'],reference,sound,20,25)
    render(bins['factory'],default,sound,20,25)
    assert digest(reference)==digest(default), f'Factory {sound} changed'
    report['proofs']['factory-'+sound+'-unchanged']=True
    if sound=='all-tide':
        candidate=out/'proof-all-tide-candidate.wav'
        render(bins['candidate'],candidate,sound,20,25)
        candidate.unlink()
        report['proofs']['candidate-eight-tide-voices-healthy']=True
    if sound in ('warm','silk','scatter'):
        candidate=out/f'proof-{sound}-candidate.wav'
        render(bins['candidate'],candidate,sound,20,25)
        assert digest(reference)==digest(candidate), f'Excluded sound {sound} changed'
        report['proofs']['candidate-'+sound+'-unchanged']=True
        candidate.unlink()
    reference.unlink();default.unlink()
repeat=out/'repeat.wav'
render(bins['candidate'],repeat,'chord',40,25)
assert digest(repeat)==digest(out/'chord-B-raw.wav'), 'Candidate not deterministic'
repeat.unlink();report['proofs']['deterministic-candidate']=True

for sound in ('single','chord','mix','all-tide'):
    results={}
    for label,binary in (('A',bins['baseline']),('B',bins['candidate'])):
        trials=[render(binary,'bench',sound,20,25) for _ in range(3)]
        results[label]={key:float(np.median([t[key] for t in trials])) for key in ('instructions_per_frame','ns_per_frame','voices')}
    results['instruction_delta_percent']=100*(results['B']['instructions_per_frame']/results['A']['instructions_per_frame']-1)
    assert results['instruction_delta_percent']<5, 'P0a host cost exceeds 5% ceiling'
    report['cpu'][sound]=results

(out/'measurements.json').write_text(json.dumps(report,indent=2)+'\n')
(out/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>Tide P0a dry comparison</title><style>body{max-width:800px;margin:40px auto;padding:20px;font:17px system-ui;background:#101b22;color:#e3e8e9}a{color:#85d2d0}audio{width:100%}section{border-top:1px solid #40505a;padding:18px 0}</style><h1>Tide P0a: dry listening comparison</h1><p>Judge static body/warmth separately from evolving movement. The still takes are identical by design. No added voices, detune, stereo or effects. Each take uses one fixed gain; breaths are not individually normalized. The default patch is unchanged.</p><p>Short takes first; long takes test five minutes of movement. A always precedes B. No listening approval has been inferred from these measurements.</p>'+''.join(rows)+'<p><a href="measurements.json">Measurements and invariant proofs</a></p>')
print('P0a listening pack, baseline proofs and CPU checks passed:',out,flush=True)
