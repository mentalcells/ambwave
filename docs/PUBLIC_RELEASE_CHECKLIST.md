# AmbWave 0.1.0 public MVP checklist

Scope agreed 2026-10-16: source, downloadable FM-1 firmware, browser demo.
This is the launch checklist; historical experiment notes keep their original names.

## Prepare locally

- [x] Rename the current product UI, browser, guides, issue templates and build entry point to AmbWave.
- [x] Freeze the accepted FM TIDE source and four macros; no additional sound redesign.
- [x] State actual MVP features and limits, especially shared voices/effects and missing scene recall.
- [x] Retain Felucca authorship, GPL and all third-party notices; clearly identify the fork.
- [x] Document installation, backups, rollback and recovery without linking an upstream installer as AmbWave.
- [x] Audit current files and reachable history for credential patterns and private artifacts.
- [x] Build the branded firmware; run required audio, CPU, storage and UI regressions.
- [x] Build AMBIENT_POC=1 browser code and compare its audio with native output.
- [x] Prepare a versioned firmware + corresponding-source + licence bundle with checksums and provenance.
- [x] Prepare a static public site with browser demo, downloads, quick start, source and credits.
- [x] Verify local links/downloads, fresh browser audio start, saved-state isolation and device-size layout.
- [x] Record validation and remaining manual checks in MVP_RELEASE.md.

## Final publication steps

- [x] Choose mentalcells as the candidate public identity and confirm the single-account strategy; see PUBLIC_IDENTITY.md.
- [x] Configure mentalcells and the verified private commit email; prepare a clean source history with the earlier development history kept private.
- [ ] Confirm the final branded package on hardware (the accepted Tide synthesis was already installed and selected by the user).
- [ ] Review and commit the updated candidate on codex/ambwave-mvp; prepare the matching clean-source bundle (the earlier candidate was committed at handoff).
- [x] Rename the retained account to mentalcells and repository to `ambwave` with approval; update origin and current source/support URLs. Preserve upstream.
- [ ] Integrate the release commit into the intended default branch and tag v0.1.0.
- [ ] Review all remote branches, tags, releases and attachments before making the repository public.
- [ ] Change repository visibility and verify anonymous source/licence access.
- [ ] Publish the versioned assets and checksums as a GitHub release.
- [ ] Enable static hosting and deploy the prepared site; verify its public URL and asset paths.
- [ ] Listen through the public browser demo and complete a hardware install/recovery smoke check.
- [ ] Add the verified live demo and release URLs to README, then announce.

Publication is a final review step. A prepared local bundle is not a published
release. The existing repository URL stays valid until the remote rename.

## Follow-up corrections — 2026-10-16

- [x] Record mentalcells as the chosen public maintainer identity; retain accurate current URLs pending migration.
- [x] Prepare visible browser levels/mutes and concise TIDE/PULSE onboarding; preserve hardware HOME controls and browser save namespace.
- [x] Correct Space 0 and macro/version spacing in current product guides, site text and release notes.
- [x] Prepare the exact-package browser installer with SHA-256 checking, backup/confirmation gates, progress, restart checks and trusted-package rollback using unchanged Felucca update modules.
- [x] Inspect the live Firmware Hub submission and installation information; record verified facts and unknown requirements in FIRMWARE_HUB.md.
- [x] Accept the revised mix after listening and adopt raw levels 82/80/94/80.
- [x] Complete fresh firmware/browser/audio validation and local candidate packaging of the adopted defaults.
- [x] Verify the SPACE audition setting does not become a dry-only build: fresh startup uses 50%, and Space 0 remains genuinely dry. The user clarified this concern; no SPACE redesign is requested.
- [x] Complete local browser layout/control verification and fresh release integrity checks for this follow-up (cross-browser/touch and hardware checks remain).
- [ ] Verify real hardware installation/recovery with explicit current-conversation approval.
- [ ] Complete clean repository verification and final publication review.

The new mentalcells/ambwave repository begins with a clean source snapshot and
remains private. Original development history is retained separately. Publication
requires explicit approval; old pull request references are not migrated.
Listening feedback found PULSE slightly too loud in the first mix candidate.
A second audition lowers only PULSE from raw level 100 to 94 (about 3 dB).
The user accepted revision 2; firmware defaults now use 82/80/94/80.
The original listening files remain in build/ambwave-mix-audition-v2.

## Local validation evidence — 2026-10-16

Hardware build: image 441300 bytes; data+BSS 89932/98304; pool 330208/344064;
package identity FM-1_900. `tools/test-macos.sh`: ALL HOST TESTS PASSED, 92
golden renders unchanged, target budgets and storage/UI checks pass. Optional
DaisySP reference tests were skipped because its checkout is absent.
The AMBIENT_POC=1 emulator/native chord, layered mix and WARM/SILK checks pass.
Release validation checks actual download hashes, source, notices and local links.
A fresh local browser tab showed the AMBWAVE splash, booted TIDE, reached Space 0,
latched and released notes. Node checks cover saved-state round trips; full
real-browser save/reload, touch and cross-browser listening remain launch checks.
The earlier installed Tide source was accepted by the user; the newly branded
package has not been flashed. No new hardware quality claim is inferred from
software checks. Public hosting and anonymous access remain unverified until deployment.
