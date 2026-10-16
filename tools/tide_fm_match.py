#!/usr/bin/env python3
"""Fixed-gain matching for the grounded FM patch audition; no audio effects."""
import array
import json
import math
from pathlib import Path
import sys
import wave

root = Path(sys.argv[1] if len(sys.argv)>1 else 'build/tide_fm_audition')
names = tuple(sys.argv[2:4]) if len(sys.argv)>=4 else ('original', 'candidate')
report = {}
for suffix in ('', '-single'):
    takes = []
    for name in names:
        stem = name + suffix
        with wave.open(str(root / (stem + '.wav'))) as wav:
            sr = wav.getframerate()
            audio = array.array('h', wav.readframes(wav.getnframes()))
        if sys.byteorder != 'little': audio.byteswap()
        held = audio[8*sr*2:20*sr*2]
        rms = math.sqrt(sum(x*x for x in held) / len(held))
        gain = 6000 / rms
        peak = max(map(abs, audio))
        if peak * gain >= 32767: raise ValueError(f'{stem}: matching would clip')
        scaled = array.array('h', (round(x*gain) for x in audio))
        if sys.byteorder != 'little': scaled.byteswap()
        takes.append(scaled.tobytes())
        with wave.open(str(root / (stem + '-matched.wav')), 'wb') as wav:
            wav.setparams((2, 2, sr, 0, 'NONE', 'not compressed'))
            wav.writeframes(takes[-1])
        report[stem] = dict(raw_rms=rms, gain_db=20*math.log10(gain), matched_rms=6000,
                            matched_peak=round(peak*gain), notes=[50] if suffix else [50,57,65,72])
    with wave.open(str(root / ('AB' + suffix + '.wav')), 'wb') as wav:
        wav.setparams((2, 2, sr, 0, 'NONE', 'not compressed'))
        wav.writeframes(takes[0]); wav.writeframes(bytes(sr*4)); wav.writeframes(takes[1])
(root / 'levels.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
