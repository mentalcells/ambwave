#!/usr/bin/env python3
"""P0b offline dry A/B(/C) auditions. Requires NumPy; never accesses a device."""
import hashlib
import html
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import wave
import numpy as np

labels=sys.argv[2] if len(sys.argv)>2 else 'ABC'
if labels not in ('ABC','AD'):raise SystemExit('Comparison must be ABC or AD')
out=Path(sys.argv[1] if len(sys.argv)>1 else 'build/p0b/listening')
out.mkdir(parents=True,exist_ok=True)
if any(out.iterdir()):raise SystemExit('Refusing to replace an existing audition: '+str(out))
binary=Path('build/p0b/render')
report={'baseline':'828ceaa original Tide at Motion0/Space0',
        'variants':{'A':'Original 1:1 FM pair and second harmonic',
                    'B':'2:1 modulator ratio, modulator OL72, second harmonic OL78',
                    'C':'B plus quiet fundamental carrier OL82, one native detune step',
                    'D':'A unchanged plus quiet fundamental carrier OL82, one native detune step'},
        'matching':'One fixed gain per entire take, RMS seconds 8..release = 6000/32768',
        'takes':{},'cpu':{},'proofs':{}}

def render(label,path,sound,seconds):
    env=dict(os.environ,TIDE_VOICING=str('ABCD'.index(label)))
    p=subprocess.run([str(binary),str(path),sound,str(seconds),'0','0'],env=env,text=True,capture_output=True)
    if p.returncode:raise RuntimeError(p.stdout+p.stderr)
    stats=json.loads(p.stdout)
    if sound=='all-tide':assert stats.get('max_voices',stats.get('voices'))==8
    return stats

def read(path):
    with wave.open(str(path),'rb') as w:
        assert (w.getframerate(),w.getnchannels(),w.getsampwidth())==(44100,2,2)
        return np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,2)

def write(path,samples):
    with wave.open(str(path),'wb') as w:
        w.setparams((2,2,44100,0,'NONE','not compressed'));w.writeframes(np.asarray(samples,dtype='<i2').tobytes())

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

rows=[]
for name,sound,hold in (('single','single',30),('chord','chord',30),('high','high',30),('chord-long','chord',90)):
    matched=[]
    for label in labels:
        raw=out/f'{name}-{label}-raw.wav';stats=render(label,raw,sound,hold)
        samples=read(raw);held=samples[8*44100:hold*44100].astype(float)
        rms=float(np.sqrt(np.mean(held**2)));gain=6000/rms
        scaled=np.rint(samples.astype(float)*gain);assert abs(scaled).max()<32767
        write(out/f'{name}-{label}-matched.wav',scaled)
        block=samples[12*44100:12*44100+65536,0].astype(float)
        power=abs(np.fft.rfft(block*np.hanning(len(block))))**2
        hz=np.fft.rfftfreq(len(block),1/44100);total=power[(hz>=20)&(hz<20000)].sum()
        bands={f'{lo}-{hi}Hz':float(power[(hz>=lo)&(hz<hi)].sum()/total) for lo,hi in ((20,80),(80,300),(300,1500),(1500,8000))}
        stats.update(raw_rms=rms,matching_gain_db=float(20*np.log10(gain)),matched_peak=int(abs(scaled).max()),
                     matched_rms=float(np.sqrt(np.mean(scaled[8*44100:hold*44100]**2))),bands=bands,sha256=sha(raw))
        assert stats['limiter_max_db']==0, 'Audition source must not gain richness from limiter activity'
        report['takes'][name+'-'+label]=stats;matched.append(scaled)
        print(name,label,'RMS',round(rms,1),'gain dB',round(stats['matching_gain_db'],2),flush=True)
    gap=np.zeros((44100,2))
    pair='AD' if labels=='AD' else 'AB'
    write(out/f'{name}-{pair}.wav',np.concatenate((matched[0],gap,matched[1])))
    extra=''
    if labels=='ABC':
        write(out/f'{name}-BC.wav',np.concatenate((matched[1],gap,matched[2])))
        extra=f'<p>Optional: B → C with quiet additional carrier</p><audio controls preload="none" src="{name}-BC.wav"></audio>'
    description='A original → D original plus quiet carrier' if labels=='AD' else 'A original → B revised voicing'
    rows.append(f'<section><h2>{html.escape(name)}</h2><p>Motion0 / Space0. Each take: {hold}s held + 12s release. One-second gap.</p><p>{description}</p><audio controls preload="none" src="{name}-{pair}.wav"></audio>'+extra+'<p>'+ ' · '.join(f'<a href="{name}-{label}-matched.wav">{label} separately</a>' for label in labels)+'</p></section>')

# Independent reference proof against the pristine renderer, not a rebuilt recipe.
baseline=Path('build/p0b/baseline-render')
if not baseline.exists():raise SystemExit('Build the pristine renderer with tools/tide-voicing-compare.sh first.')
reference=out/'reference.wav'
subprocess.run([str(baseline),str(reference),'chord','30','0','0'],check=True,capture_output=True)
assert sha(reference)==sha(out/'chord-A-raw.wav');reference.unlink()
report['proofs']['A-identical-to-pristine-baseline']=True
candidate=labels[1]
repeat=out/'repeat.wav';render(candidate,repeat,'chord',30)
assert sha(repeat)==sha(out/f'chord-{candidate}-raw.wav');repeat.unlink()
report['proofs'][candidate+'-deterministic']=True

for sound in ('single','chord','all-tide'):
    row={}
    for label in labels:
        trials=[render(label,'bench',sound,20) for _ in range(3)]
        row[label]={key:statistics.median(t[key] for t in trials) for key in trials[0]}
        if sound=='all-tide':
            path=out/f'stress-{label}.wav';stats=render(label,path,sound,20);path.unlink()
            report['proofs'][label+'-eight-voice-health']=stats
    for label in labels[1:]:row[label+'-vs-A-percent']=100*(row[label]['instructions_per_frame']/row['A']['instructions_per_frame']-1)
    report['cpu'][sound]=row
(out/'measurements.json').write_text(json.dumps(report,indent=2)+'\n')
intro='<h1>Tide A/D: preserve the warm source</h1><p>A is original Tide. D keeps A’s original FM ratio and harmonic balance, adding only one quiet slightly detuned fundamental carrier. Judge body and grounding, and whether the extra layer produces thickness or merely beating.</p>' if labels=='AD' else '<h1>Tide P0b: static fullness</h1><p>A is current Tide. B revises the FM harmonic structure using the same three active operators. Start with A/B; judge body, warmth and definition at matched loudness.</p><p>C adds a quiet carrier to B.</p>'
(out/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>Tide dry voicing comparison</title><style>body{max-width:820px;margin:35px auto;padding:20px;font:17px system-ui;background:#111c22;color:#e2e8e9}audio{width:100%}a{color:#89d2cb}section{border-top:1px solid #45545a;padding:16px 0}</style>'+intro+'<p>No LFO, breathing or spatial effects. A fixed detune can cause beating despite Motion0. Nothing has been installed or made default.</p>'+''.join(rows)+'<p><a href="measurements.json">Measurements and checks</a></p>')
print('P0b auditions and baseline/resource checks passed.',flush=True)
