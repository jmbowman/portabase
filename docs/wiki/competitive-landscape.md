# Competitive Landscape

Research notes on other data-management apps that might overlap with PortaBase's niche, gathered while working through the "verify there still isn't a suitable replacement" step of [roadmap.md](roadmap.md). Raw material for a future "why PortaBase" page on the updated web site — not polished copy yet.

Research date: 2026-07-06. This space moves fast (new local-first tools, AI-assisted app builders, etc.), so re-verify pricing/feature claims before publishing anything derived from this doc.

## PortaBase's Niche, for Reference

* A file-based (not server/account-based) tabular data manager: the data file is a document you own, like a spreadsheet, not a hosted service.
* No SQL or query-language knowledge required to define schema, filters, sortings, or calculated fields.
* Schema can change at any time, and everything derived from it (filters, sortings, calculated fields) auto-updates rather than breaking.
* Genuinely cross-platform desktop + mobile, historically down to very small/low-power devices.

The gap PortaBase fills is the intersection of all four of these. Most competitors satisfy two or three but not all.

## Closest Competitors Found

### HanDBase (DDH Software)

* What it is: A relational, no-SQL personal database manager with custom schemas, dating back to the Palm/Pocket PC era, still cross-platform across desktop and mobile today.
* Status: Actively maintained — v4.9.087 shipped 2025-06-04. Not abandoned.
* Where it falls short: Proprietary/paid per-platform pricing model; UI and overall feel are generally described as dated. This is the longest-standing direct competitor to PortaBase and is presumably one you've already evaluated in the past — the main news here is that it's still alive, not that it's caught up.
* Sources: <https://www.ddhsoftware.com/handbase.html> , <https://sourceforge.net/software/product/HanDbase/> , <https://sourceforge.net/software/compare/HanDbase-vs-Memento-Database-vs-kintone/>

### Memento Database

* What it is: The most actively modernized alternative — flexible custom fields, cross-platform including desktop, offline-first with sync.
* Status: Actively developed, subscription pricing (~$4/user/month).
* Where it falls short: Built around an account + cloud-sync model as the primary experience, not "a file you own and can treat like any other document." That's a philosophical mismatch with PortaBase's core value proposition, not just a missing feature.
* Sources: <https://www.g2.com/products/memento-database/reviews> , <https://sourceforge.net/software/product/Memento-Database/>

### Tap Forms

* What it is: A macOS/iOS personal database/record-keeping app.
* Where it falls short: Flat record-keeping only — not relational, and weaker on calculated fields than HanDBase or PortaBase per user comparisons.
* Sources: <https://justuseapp.com/en/app/291405311/tap-forms-organizer/reviews>

### Airtable-style / "database app builder" tools (Baserow, NocoDB, NocoBase, Grist, Notion, SmartSuite, etc.)

* What they are: The dominant category that shows up in any 2026 search for "Airtable alternative" — open-source self-hosted (Baserow, NocoDB, NocoBase), spreadsheet-first (Grist), or all-in-one workspace (Notion, SmartSuite) tools.
* Where they fall short: All require either a server/self-hosting setup or a hosted account, and are built for team collaboration rather than a single portable data file. Wrong shape entirely for PortaBase's use case — worth naming in a competitive-landscape page mainly to explain why they don't count, since they dominate the search results for this space.
* Sources: <https://lovable.dev/guides/airtable-alternatives-custom-apps> , <https://baserow.io/blog/best-airtable-alternatives>

### Obsidian Bases (New Core Plugin, Early 2026)

* What it is: A genuinely local-first, file-based feature — turns the YAML properties already in a vault's markdown notes into table/card/list/map views, no proprietary storage format.
* Where it falls short: Scoped to note metadata within an Obsidian vault, not a standalone data file. No sign of encryption, calculated fields, or arbitrary relational schema support comparable to PortaBase. A PKM feature, not a data-grid app — but worth watching since it's the newest thing in this space that shares PortaBase's local-first, plain-file philosophy.
* Sources: <https://obsidian.md/help/bases> , <https://got.md/obsidian-bases/>

## Conclusion

No drop-in replacement found. Nothing combines all four of PortaBase's defining traits (portable file, no-SQL, live-updating schema-derived artifacts, true cross-platform reach). HanDBase is the closest in spirit but dated and proprietary; Memento is the most modern but account/cloud-first; Obsidian Bases is the most philosophically aligned newcomer but not built for this kind of structured data. Proceeding with the revival roadmap is reasonable.

## Why Hasn't Anyone Else Built This?

Worth addressing directly on a future "why PortaBase" page, since it's a fair question: after 25 years, why hasn't a small team with modern tooling filled this gap, given how many users have said they value it?

The likely answer is economics, not lack of demand. A "no-SQL, live-updating schema, portable file you own" product structurally rules out the two things that fund a team: a subscription and a hosted/cloud data model. Needing recurring revenue pushes a team toward accounts and hosting (Airtable, Memento, Notion) — exactly what this kind of user values PortaBase for *not* requiring. So the market isn't short on people who see the value; it's short on a way to get paid for building it that doesn't destroy the value. That selects for solo maintainers scratching their own itch (this project, HanDBase's original author) rather than funded teams, regardless of how much faster a team could build it today.

There's also a spec problem: "filters and calculated fields keep working correctly when the schema changes" is the kind of requirement that's obvious once you've lived with the pain of it breaking, but hard to write as a contracted spec for someone else to build — part of why tools like this tend to get built by people solving it for themselves.

This is one instance of a broader, unfortunately-more-true-than-it-should-be dichotomy: open source projects tend to have either impact or revenue, but rarely both. Software that genuinely helps people at this scale often can't sustain a business model without compromising the properties that make it valuable in the first place.

One concrete implication: treat "local file ownership with no required account/subscription" as a load-bearing, identity-level constraint when evaluating future business-model or sync/cloud proposals (e.g. the planned Turso backend's sync options), not as a negotiable feature. Optional cloud sync on top of a still-fully-functional local file is fine; pivoting the core product toward a subscription or hosted-service model to fund development is not.
