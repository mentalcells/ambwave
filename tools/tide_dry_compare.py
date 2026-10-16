#!/usr/bin/env python3
"""Level-match dry Tide renders; report sustain movement without judging tone.
Run after tests/tide_dry_test.c has rendered before/ and after/.
"""
import array
import json
import math
from pathlib import Path
import sys
import wave

root = Path(sys.argv[1] if len(sys.argv) > 1 else 'build/tide_harmonics_demo')
report = {}
for name in ('default-chord', 'single-source', 'bright-register'):
    matched = []
    for version in ('before', 'after'):
        with wave.open(str(root / version / (name + '.wav')), 'rb') as wav:
            rate = wav.getframerate()
            audio = array.array('h', wav.readframes(wav.getnframes()))
        if sys.byteorder != 'little': audio.byteswap()
        held = audio[8*rate*2:20*rate*2:2]
        rms = math.sqrt(sum(x*x for x in held) / len(held))
        colours = []
        for sec in range(8, 20):
            chunk = audio[sec*rate*2:(sec+1)*rate*2:2]
            colours.append(sum((b-a)**2 for a,b in zip(chunk, chunk[1:])) / sum(x*x for x in chunk))
        report[f'{version}/{name}'] = dict(rms=rms, brightness_per_second=colours,
            last_second_peak=max(map(abs, audio[-rate*2:])), gain_db=20*math.log10(6000/rms))
        # One fixed gain per whole take, based only on settled sustain.
        scaled = array.array('h', (round(x*6000/rms) for x in audio))
        matched.append(scaled)
        if sys.byteorder != 'little': scaled.byteswap()
        with wave.open(str(root / f'{name}-{version}-matched.wav'), 'wb') as wav:
            wav.setparams((2,2,rate,0,'NONE','not compressed')); wav.writeframes(scaled.tobytes())
    # A then B with one second of silence; the two original takes stay available.
    with wave.open(str(root / f'{name}-AB.wav'), 'wb') as wav:
        wav.setparams((2,2,rate,0,'NONE','not compressed'))
        wav.writeframes(matched[0].tobytes()); wav.writeframes(bytes(rate*4)); wav.writeframes(matched[1].tobytes())
(root / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')
for key, data in report.items():
    values = data['brightness_per_second']
    print(f'{key}: held RMS {data["rms"]:.1f}, match {data["gain_db"]:+.2f} dB; '
          f'late brightness {min(values):.5f}..{max(values):.5f}; end peak {data["last_second_peak"]}')
