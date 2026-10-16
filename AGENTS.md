# Ambient firmware working agreements

This repository is a local development fork of Hügelton Instruments' Felucca
firmware for the M-VAVE FM-1.

## Product direction

- Build a focused ambient instrument, not another general-purpose groovebox.
- Prefer immediate performance controls and a small number of curated sounds.
- The initial macro vocabulary is `SHAPE`, `MOTION`, `DENSITY`, and `SPACE`.
- Reuse Felucca's existing engines and infrastructure before adding new DSP.
- Keep changes small, testable, and easy to compare with upstream Felucca.

Read `AMBIENT_PLAN.md` before proposing product or architecture changes.

## Safety

- Never flash a connected FM-1 without the user's explicit approval in the
  current conversation.
- Building a `.fwsc` package does not authorize installing it on hardware.
- Keep the bootloader, loader, recovery path, and package format unchanged
  during the first milestones.
- Do not overwrite user presets, samples, projects, or device backups.

## Development workflow

- Preserve `upstream` as the official Felucca repository.
- Make product changes on topic branches based on `ambient-prototype`.
- First prove that an unchanged upstream checkout builds and passes tests.
- Run `./build.sh` before firmware testing and `sh tests/run_tests.sh` after
  relevant changes.
- Treat CPU-budget, regression-render, storage-compatibility, and UI tests as
  product requirements, not optional checks.
- Do not update golden audio hashes unless the sound change is intentional and
  has been reviewed.



## Public identity

- The maintainer is `mentalcells`; this clean source repository is
  `mentalcells/ambwave`, private until publication is explicitly approved.
- The original development repository and local history remain private archives.
  Never mirror-push their branches, Codex checkpoints or backup refs here.
- Use repository-local Git identity only, with the verified GitHub noreply email
  and `user.useConfigOnly=true`. Never invent an email or alter global identity.
- Keep private migration audits/backups ignored and out of release archives.
- Remote renames, history changes, new-repository pushes, visibility changes,
  transfers and deletion require explicit approval. Preserve upstream credits.
- Keep the current checkout path so Codex development continues normally.
