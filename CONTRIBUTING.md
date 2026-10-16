# Contributing to AmbWave

AmbWave is a focused ambient-instrument fork of Felucca. Report AmbWave bugs and
ideas in [AmbWave issues](https://github.com/mentalcells/ambwave/issues), rather
than sending fork-specific support requests to upstream.

Include the commit/build, browser or device, the sound and playing mode, steps
to reproduce, and expected/actual behavior. Identify hardware listening reports
separately from emulator results. Remove private data from logs and backups.

Read [AGENTS.md](AGENTS.md) and [AMBIENT_PLAN.md](AMBIENT_PLAN.md) before product
changes. Keep patches small, retain recovery and storage compatibility, build
the AmbWave configuration and run the relevant tests plus the required regression
suite. Audio quality, CPU cost, voice limits and UI layouts are part of validation.
Do not update golden audio hashes to conceal an unintended sound change.

Retain original copyright notices. New source files should carry the
`SPDX-License-Identifier: GPL-3.0-only` header. Contributions are under GPL-3.0-only.
AI-assisted contributions are welcome when their author understands the changes
and provides appropriate validation.

The original upstream contribution guide is preserved in
[CONTRIBUTING_FELUCCA.md](CONTRIBUTING_FELUCCA.md).
