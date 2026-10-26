# Platform release-set metadata

Darling's generated VersionMap stops in spring 2021. New Intel Xcode helpers call the per-image SDK/minimum-version predicates with later release sets, which previously hit an assertion.

DarlingVersionMap.h contains numeric compatibility metadata, with no Apple executable instructions. The generator reads a locally supplied Intel dyld, identifies its two compiled sVersionMap arrays by symbol, derives their stride/count from the matching mapFromVersionSet routine, verifies that both arrays are byte equal, and emits the six fields used by this dyld generation. The selected reference has61 entries through the26.6 release series. The platform field order is checked against the mapper disassembly and known earlier platform versions. Public Apple AvailabilityVersions data independently documents earlier sets; its current public DSL ends in2023 and is not the source of invented later mappings.

The caller SDK and minimum versions still come from real Mach-O load commands. Unknown future sets return no matching platform, consistent with the current Apple open-source dyld implementation. This table is used for both per-image predicates and the executable fast-path equivalence calculation.

The reference binary remains a private user-supplied input. To regenerate into a fresh candidate file, run darling/scripts/extract-version-map.py with --source, --output, and --report. The report pins the input/table/output SHA256 and records the mapper disassembly. Existing destinations are refused.

Native Linux/Darling controls used three genuinely loaded Mach-O dylibs with11/11,14/14, and26/26.5 minimum/SDK versions, ten release-set boundaries, explicit platform equivalence, wrong platform rejection, unknown future sets, and main-executable fast paths. This does not replace or rebuild the pinned main dyld2 loader ABI.
