# The Firmware Hub preparation

Inspected the live [FM-1 Firmware Hub](https://thefirmwarehub.com/) in a browser
on 2026-10-16. The web fetch tool failed, but the browser loaded the catalogue,
submission section, FAQ and expanded installation explanation. No device was
connected through the site and no submission was sent.

## Verified public requirements and behavior

- Submission: send the firmware name, repository and description to the contact
  listed under "Your firmware". A public, free GPL-3.0 source project and a
  `.fwsc` release on GitHub are requested.
- Attribution: cards identify the author and link the project, release and
  source. Browser demos and editors can have separate links.
- Updates: the FAQ says the hub checks repositories nightly for their newest
  release and marks betas. It checks for an FM-1 package with an update loader.
- Distribution: the hub mirrors release packages unchanged and keeps a source
  copy alongside them. Its installer checks SHA-256 against its catalogue.
- Installation: Chrome or Edge, USB MIDI; the running firmware stages the
  package loader, which writes the firmware. The site advises backups first.

## AmbWave preparation and remaining verification

Use **AmbWave**, by **mentalcells**, with Felucca attribution and GPL-3.0-only
plus all component notices. Publish the reviewed `.fwsc`, corresponding source,
SHA256SUMS, manifest and notices against the same release tag. Retain FM-1_900
and the existing Felucca loader/package format. Describe shared voices/effects
and incomplete scene recall; link the actual demo and installation guide.

The public page does **not** specify a submission manifest schema, a required
asset filename, a separate package ID, cross-origin hosting headers or a formal
acceptance test. Do not invent these. Since the hub mirrors assets, its stated
flow does not establish a requirement for CORS on AmbWave's own site. Confirm
asset selection, prerelease selection and any additional metadata with the hub
maintainer at submission time. Keep versioned assets immutable. A locally valid
package does not prove hub acceptance or end-to-end hardware compatibility.

Pending: verified public account/repository and release URLs, branded hardware
check with explicit flash approval, final package checksums, hub submission
approval, catalogue acceptance and verification of the mirrored download hash.
