[![docs](https://github.com/DASAElektronik/T27/actions/workflows/docs.yml/badge.svg)](https://github.com/DASAElektronik/T27/actions/workflows/docs.yml)
# Ternary Computer (T27) — Community Edition

> **Kurzfassung (DE):** Freies, patent-unbelastetes Projekt für Ternärlogik (−1/0/+1 oder 0/1/2). 
> Code/HDL: Apache‑2.0 · Hardware-Designs: CERN‑OHL‑P‑2.0 · Doku/Media: CC‑BY‑4.0. 
> Beiträge willkommen!

> **EN Summary:** Open, patent-friendly ternary logic project. 
> Code/HDL: Apache‑2.0 · Hardware: CERN‑OHL‑P‑2.0 · Docs/Media: CC‑BY‑4.0. Contributions welcome!

---

## ✨ Vision
Energy-efficient, accessible ternary building blocks (cells, adders, encoders/decoders, reference designs) 
to enable research and hobbyist-grade prototypes, with clear design choices that avoid third‑party patents.

## 🗂 Repository Layout (recommended)
```
/src        # software, tooling, scripts (Apache-2.0)
/hdl        # RTL/HDL (Verilog/VHDL/SystemVerilog) (Apache-2.0)
/hardware   # schematics, PCB, mechanics (CERN-OHL-P-2.0)
/docs       # specs, figures, papers (CC-BY-4.0)
/examples   # demos, test designs
```

## 🔐 Licensing
- See **LICENSE-ROUTING.txt** for the multi-license setup.
- Code & HDL → **Apache-2.0** (with patent grant)
- Hardware design files → **CERN‑OHL‑P‑2.0**
- Documentation & media → **CC‑BY‑4.0**

Add SPDX headers to files; examples in **SPDX-EXAMPLES.md**.

## 🧭 Scope & Patent Safety
We **avoid** specific +1/−1 base-cell implementations with particular preprocessing and 
multi-threshold transistor topologies known from certain filings.
See **SCOPE-NON-INFRINGMENT-NOTE.md** for the explicit design-around statement.

## 🚀 Quickstart
```bash
# clone & prepare
git clone <your-repo-url> && cd <repo>
# optional: set up Python/Node toolchains for generators/sim
# run example simulation
make sim EXAMPLE=adder3
# synth or PCB flow depends on your EDA setup (documented in /docs)
```

## 🧪 Tests
- Unit/logic tests with your preferred framework (document in `/docs/test-setup.md`)
- CI: lint + sim + build (GitHub Actions example coming soon)

## 🤝 Contributing
Please read **CONTRIBUTING.md** (DCO + SPDX). 
Open a discussion or issue before large changes. Be kind: **CODE_OF_CONDUCT.md** applies.

## 🛡 Security
Report potential vulnerabilities as described in **SECURITY.md**.

## 📣 Citing
If you publish results based on this project, please cite the repo and (when available) the Zenodo DOI:
```
@software{t27_ternary_computer,
  title        = {Ternary Computer (T27)},
  year         = {2025},
  url          = {<repo-url>},
  version      = {<tag-or-commit>},
  doi          = {<zenodo-doi-if-any>}
}
```

## 🗺 Roadmap (sketch)
- [ ] Core cell library (INV3, MUX3, FA3)
- [ ] Level generation (ladder + window comparator)
- [ ] Encoders/decoders (dual-line digital encoding)
- [ ] Reference designs (ALU3, small CPU core)
- [ ] Boards & fixtures (evaluation, measurement)
- [ ] Docs & reproducible measurements

## 🙏 Acknowledgments
Thanks to all contributors. See **NOTICE** for attributions.
