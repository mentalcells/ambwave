# Public MVP preparation

Prepared 2026-10-15. This is a release work record, not an announcement that
anything has been published. Repository: `mentalcells/ambwave`; the visibility
was verified as PRIVATE, with default branch `ambient-prototype`.

## Identity and documentation prepared

- Product name: Hasake / חסקה. TIDE remains the harmonic sound identity, with
  TIDE WARM and TIDE SILK variations. Do not rename the product to Felucca or
  change device/package identifiers merely for branding.
- The root README describes Hasake and the actual MVP limits. The upstream
  README and contribution guide remain as README_FELUCCA.md and
  CONTRIBUTING_FELUCCA.md, retaining their notices and working root-relative links.
- QUICK_START.md covers browser gestures, sound selection, macros, mixing,
  Hold/Scatter/Sequence, clearing and shared resources.
- LICENSING.md identifies the fork and retains all upstream component notices.
  LICENSE and LICENSES/ remain unchanged. The browser credits Felucca/X0X/fonts
  and links to Hasake's source and notices. No borrowed commercial pad-bank
  material was added; the new patches derive from Felucca's own presets.

## Before changing repository visibility

1. Finish hardware listening for the release candidate, including macro sweeps,
   release tails and a mixed scene. Record the tested commit/package digest.
2. Integrate the chosen topic commits into the intended release/default branch;
   verify the public README matches that exact code and remove development-only
   branch wording. Preserve `upstream` as the official Felucca remote.
3. Review the complete history, branches, tags, releases and attachments that
   will become public. The current tracked-file check found no private-key,
   GitHub-token, AWS-key or OpenAI-key pattern matches and no credential-like
   filenames. Ignored build/, .venv/ and .toolchains/ are not tracked. This is a
   limited pattern check. An additional scan of all 908 reachable historical
   blobs (42,785,009 bytes) also found no matches for those credential patterns;
   filename matches for "backup" were the backup implementation/tests, not user
   backups. Neither scan guarantees absence of all sensitive data. Existing
   docs include local paths and development records; review their suitability.
4. Verify `.fwsc` redistribution and matching source/licence bundles, including
   the tracked rollback package. Preserve upstream and third-party notices;
   include the unmodified SDK component licences with a hardware package.
5. Review the about screen, funding links and source references. Hasake's issue
   templates now direct its reports and questions to this repository. Device
   CREDITS includes the Hasake source while retaining every upstream notice.
   The inherited funding configuration still supports upstream Hügelton.
   The current device ABOUT QR still points to upstream Felucca; it has not been
   silently replaced or presented as a Hasake installer.
6. Prepare a versioned release with matching source, build instructions, tested
   `.fwsc`, digest, licences and clear recovery instructions. Keep the previous
   known-good package available. Do not flash as part of publishing.
7. Change visibility at the agreed final release step, then verify the source
   and notices are readable without authentication. This has not been done.

## Browser publication follows separately

Choose hosting, build explicitly with AMBIENT_POC=1, and publish a reproducible
static bundle with source and notices. Verify audio start, computer/touch input,
asset URLs, safe saved-state handling, and supported browsers at the public URL.
Do not advertise the existing upstream installer as a Hasake installer. The
local emulator is available now; no public hosting URL is claimed yet.
