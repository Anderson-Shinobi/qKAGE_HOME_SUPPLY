# Changelog

## [v1.2.1-beta]

Structural consistency patch for operational synchronization between Dashboard, Estoque, Compras and Cofrinhos.

### Fixed

- Compras now resolves the persisted stock CSV consistently when the Qt app is launched from `build/`.
- Compras parser now tolerates shopping-list payloads with or without estimated cost columns.
- Critical autonomy at the configured one-month boundary is now treated consistently as `CRÍTICO`.
- Cofrinhos now resolves `piggybanks.csv` consistently when the Qt app is launched from `build/`.
- Cofrinhos table binding now uses a robust row parser and recalculates percentage from persisted values.
- Dashboard now uses the same resolved stock and piggy bank data paths as the operational pages.

### Improved

- Added INFO/WARNING logs for Compras and Cofrinhos synchronization.
- Estoque, Dashboard, Compras and Cofrinhos now read the same persisted data source in the Qt flow.
- Reduced false empty states caused by relative-path drift.

## [v1.2.0-beta]

Operational beta focused on turning every primary Qt module into a coherent visual demo for screenshots and product presentation.

### Added

- dynamic shopping engine
- piggybank visualization
- investment projection system
- dashboard operational integration
- visual polish
- logs cleanup
- shopping priorities with CRÍTICO and ATENÇÃO states
- estimated purchase total
- piggy bank progress bars
- investment projection chart for 12-month simulated growth

### Improved

- Compras now surfaces actionable replenishment data instead of an empty state.
- Cofrinhos now displays real CSV-backed financial goals.
- Investimentos now presents a complete demonstration projection flow.
- Dashboard now connects purchases, piggy banks and investment projections into the main operational view.
- Visual hierarchy is more consistent across Qt pages for GitHub and LinkedIn screenshots.

### Fixed

- Removed construction placeholders from Cofrinhos and Investimentos.
- Cleaned stale parsing ERROR entries from demo logs.
- Reduced blank operational areas in primary navigation.

## [v1.1.0-beta]

Beta release focused on the Qt operational dashboard, visual consistency, release metadata and validation coverage.

### Added

- InvestmentsPage
- OperationalChartWidget
- gráfico temporal operacional
- gauges operacionais
- status operacional compacto
- feedback banner integrado
- sidebar refinada
- branding "qKage Home Supply"
- AboutDialog profissional
- roadmap visual de investimentos
- Operational Temporal Chart

### Improved

- hierarquia visual do Dashboard
- alinhamento dos InfoCards
- clipping/layout overflow corrections
- consistência visual Qt
- spacing/padding refinado
- status synchronization
- UX operacional
- integração dos Controllers
- renderização dos gauges

### Fixed

- problemas de clipping visual
- overflow de labels
- inconsistência do status operacional
- parsing de indicadores
- sincronização do Dashboard
- estados residuais ERROR

### Infrastructure

- smoke tests refinados
- Qt6 validation
- CMake modular build
- tests coverage expandida
- release preparation
- repository organization

## v1.0.0-alpha - Phase 2 Release

Initial professional release preparation for `qKAGE_HOME_SUPPLY`.

### Added

- Initial layered architecture with CLI, Controllers, Services and Core boundaries.
- CSV-based local persistence.
- CLI dispatcher with command validation.
- Stock flows: list, add, edit, consume and safe remove.
- Controller adapters for stock, finance, report and system workflows.
- Service layer for stock, finance, reports, backup, logs, config and integrity checks.
- Qt Widgets GUI bootstrap.
- Dashboard Qt page with InfoCards, StatusBadge, FeedbackBanner and QPainter gauges.
- StockPage visual CRUD integration.
- ShoppingPage integration with controller-driven shopping list.
- ReportsPage integration with monthly report and Markdown export.
- LogsPage integration with filtered operational logs.
- SettingsPage integration with config display.
- AboutDialog with project identity, author and version.
- ThemeManager with dark visual identity and operational orange accent.
- StatusBadge and FeedbackBanner reusable widgets.
- Backup manager and integrity check flows.
- Operational logs and log filtering.
- Markdown reports.
- Qt Resource System assets for icons and temporary logo.

### Notes

- Roadmap is closed at Phase 2 for this release.
- SQLite is not implemented.
- Google Drive integration is not implemented.
