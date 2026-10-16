# Publishing the reviewed AmbWave MVP

These are maintainer instructions for the final publication step, not commands
that run during a normal build. Preparation tools never flash or publish.

## Build and review

1. Complete PUBLIC_RELEASE_CHECKLIST.md and hardware listening. Commit the
   complete source candidate on a topic branch based on ambient-prototype.
2. Run `tools/build-ambwave-macos.sh`, `tools/test-macos.sh`, and
   `tools/build-ambwave-browser.sh`. Emscripten may need
   `EM_CACHE="$PWD/.toolchains/emscripten-cache"` on the development Mac.
3. Run `python3 tools/ambwave_release.py audit --history` and review remote
   refs/releases/attachments as well as local scan findings.
4. Run `python3 tools/ambwave_release.py package` from a clean reviewed commit.
   A dirty local preview requires `package --candidate`; it is explicitly marked
   and includes its exact working source rather than falsely claiming HEAD.
5. Run `python3 tests/ambwave_release_test.py build/ambwave-0.1.0`.
   Review manifest.json, SHA256SUMS, notices, and the source archive. The output
   directory is never overwritten automatically; preserve or move older candidates.
6. Serve `build/ambwave-0.1.0/site` locally and verify the demo and downloads.
   Re-run the AMBIENT_POC=1 native/WebAssembly comparison after any DSP change.

Build stamps reject outdated source/artifact combinations and product wrappers
select AMBIENT_POC=1. Never substitute the generic browser build, which intentionally
boots the upstream-compatible configuration. Normal host tests can overwrite
build/emu; the product browser is preserved in build/ambwave-browser.

## Repository and release

The public identity is **mentalcells**. The current repository is
`mentalcells/ambwave`, private, starting from a clean source snapshot. Earlier
development history is kept in a separate private archive. Origin uses the
verified new repository URL. See [PUBLIC_IDENTITY.md](PUBLIC_IDENTITY.md).
Preserve the official Felucca upstream remote and all licence notices.

Integrate the reviewed topic commit into the intended default branch, tag
v0.1.0, then make the reviewed repository public and verify it anonymously.
Create the release against that exact tag. Upload the `.fwsc`, matching source
archive, complete distribution zip, checksums, manifest and licence material.
Use CHANGELOG.md as the release description, with known limits and installation
instructions. Do not upload build caches, toolchains, private backups or tokens.
Do not advertise the locally prepared candidate as a published final release.

## Static demo hosting

Deploy only the generated `site/` directory. It has relative asset URLs and a
.nojekyll marker, so it can live under a GitHub Pages project path. Enable Pages
for the reviewed repository at the final publication step and deploy this exact
artifact, or choose an equivalent static host. Do not deploy the repository root
or the build directory. The site includes downloads and all package notices.

Verify the actual HTTPS URL in a fresh browser: landing links, audio startup,
held-note playing, macro response, fresh saves/reload, mobile/touch operation,
download hashes, matching source and credits. Verify Chrome/Edge and Safari;
record unsupported cases rather than assuming browser coverage from Node tests.
Then add the real demo/release URLs to README and announce. No public URL is
claimed before that check.
