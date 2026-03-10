# Smoke Tests

## Objectif
Valider rapidement la compilabilité et la cohérence de configuration.

## Commandes recommandées
- `pio run`
- `cmake -S . -B build && cmake --build build` (si CMake est présent)

## Critères de réussite
- Build sans erreur bloquante.
- Configuration cible cohérente.
