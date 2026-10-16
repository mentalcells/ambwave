# AmbWave 0.1.0 MVP preparation

Status: private source preparation, not a public release. The firmware package
has not been installed on hardware as part of this release preparation.

## Product scope

AmbWave is an ambient instrument derived from Felucca for the M-VAVE FM-1.
The four layers are TIDE, TEXTURE, PULSE and CLOUD, with SHAPE, MOTION, DENSITY
and SPACE macros. Existing engines, storage and update formats are retained.

The accepted startup mix is raw levels 82/80/94/80. The accepted FM TIDE source
uses startup SHAPE 50%, MOTION 25%, DENSITY 50% and SPACE 50%. SPACE 0 remains
dry; an earlier dry audition was not the factory default. Browser controls
include layer faders and mutes, and saved-state round trips were checked.

## Software validation

The full host suite passed after the latest source link updates, including
unchanged golden audio renders, CPU budget, storage compatibility, UI, and
native/browser agreement. Firmware and AMBIENT_POC=1 browser builds passed.
The firmware image uses 441300 bytes; data/BSS 89932 of 98304 bytes and the
pool 330208 of 344064 bytes. Package identity remains FM-1_900.

Local release checks passed source provenance, artifact hashes, notices and
41 local links. Installer simulations cover invalid checksums, backup and
confirmation gates, unsupported browsers, normal/resume updates, rollback
selection and stopped updates. These simulations do not establish hardware
installation or recovery safety.

Private logs and older release snapshots are retained locally under ignored
build and backup directories. They are not distributed source inputs. Rebuild
release provenance from the clean repository commit before publishing.

## Repository preparation

The public source begins with one clean snapshot under mentalcells. Original
private development history is retained separately. Upstream Felucca licences,
credits, component notices and dated technical implementation notes remain.
No firmware engine, preset, storage format or update protocol is changed for
this repository migration.

## Remaining release gates

- Review the final source and regenerated release package.
- Complete browser/touch coverage and listening where still outstanding.
- Test real hardware installation, backup, rollback and recovery only with
  explicit current-conversation approval; never overwrite user device data.
- Approve repository visibility, public hosting and the release separately.
