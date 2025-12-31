# Crypto Trader Dashboard (OCaml)

This component provides a web-based control plane and monitoring dashboard for the C++ Crypto Trader system.

## Stack
-   **OCaml** (v4.14+)
-   **Dream**: A high-level, easy-to-use web framework.
-   **Dune**: Build system for OCaml.

## Getting Started

### Development
If you are inside the `nix develop` shell, you can start the server with:
```bash
dashboard
```
Or manually from this directory:
```bash
dune exec crypto_trader_dashboard --watch
```

The server will be available at `http://localhost:8080`.

## Features
-   [ ] **Monitoring**: Receive performance updates from C++ instances via `/update/performance`.
-   [ ] **Control**: Spin up new instances of `crypto_trader`.
-   [ ] **Configuration**: Edit `config.json` via the web interface.
-   [ ] **Visuals**: (Planned) Real-time charts of P&L and market data.
