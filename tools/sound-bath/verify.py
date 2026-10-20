#!/usr/bin/env python3
"""Compare imported Sound Bath scenarios with its existing WAV recordings."""
import argparse, hashlib, json, pathlib, subprocess, wave, array, math
ap=argparse.ArgumentParser();ap.add_argument('--reference',required=True,type=pathlib.Path);a=ap.parse_args()
out=pathlib.Path('build/sound-bath');out.mkdir(parents=True,exist_ok=True)
records=[]
for patch,name in [(0,'glass-bell'),(3,'bowed-metal'),(5,'bowl-variant')]:
 for scenario in ['strike','sustain','repeat','interaction','control','pressure']+(['bowl'] if patch==5 else []):
  path=out/f'{name}-{scenario}-dry.wav'; reference=a.reference/f'{name}-{scenario}-dry.wav'
  if not reference.exists(): continue  # Only compare recordings actually in the source corpus.
  info=json.loads(subprocess.check_output([str(out/'integrated'),str(patch),'0',scenario,str(path)],text=True))
  source=reference.read_bytes();actual=path.read_bytes();equal=source==actual
  with wave.open(str(reference),'rb') as f: x=array.array('h',f.readframes(f.getnframes()))
  with wave.open(str(path),'rb') as f: y=array.array('h',f.readframes(f.getnframes()))
  delta=[b-a for a,b in zip(x,y)];rms=math.sqrt(sum(d*d for d in delta)/len(delta))
  info.update(file=str(path),reference=str(reference),sha256=hashlib.sha256(actual).hexdigest(),sample_exact=equal,max_difference=max(map(abs,delta)),difference_rms=rms)
  records.append(info);print(name,scenario,'EXACT' if equal else f'DIFFERENT rms={rms:.6f}')
(out/'equivalence.json').write_text(json.dumps(records,indent=2)+'\n')
assert all(r['sample_exact'] for r in records), 'Dry equivalence failed; inspect equivalence.json'
