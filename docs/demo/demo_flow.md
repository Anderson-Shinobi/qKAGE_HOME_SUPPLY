# qKAGE_HOME_SUPPLY Demo Flow

## Demonstration Sequence

1. Present the project purpose: domestic stock control and financial logistics.
2. Show CLI help and core commands.
3. Add stock items from CLI.
4. Consume an item from CLI.
5. Generate shopping list.
6. Open the Qt GUI.
7. Navigate Dashboard, Stock, Shopping, Reports, Logs, Settings and About.
8. Export a Markdown report.
9. Show logs and backup command.

## CLI Commands

```bash
./build/qKAGE_HOME_SUPPLY help
./build/qKAGE_HOME_SUPPLY list
./build/qKAGE_HOME_SUPPLY add "Arroz" "Alimentos" 5 "kg" 1 7.50 2027-12-31 1
./build/qKAGE_HOME_SUPPLY consume "Arroz" 1 "uso mensal"
./build/qKAGE_HOME_SUPPLY shopping-list
./build/qKAGE_HOME_SUPPLY monthly-report
./build/qKAGE_HOME_SUPPLY export-report
./build/qKAGE_HOME_SUPPLY logs-show ERROR
./build/qKAGE_HOME_SUPPLY backup
```

## GUI Flow

1. Run:

```bash
./build/qKAGE_HOME_SUPPLY --gui
```

2. Dashboard: show gauges, operational summary and system status.
3. StockPage: search, filter, add, edit, consume and remove stock item.
4. ShoppingPage: show purchase suggestions.
5. ReportsPage: refresh report and export Markdown.
6. LogsPage: filter by ERROR.
7. SettingsPage: show system config.
8. AboutDialog: show project identity and author.

## Screenshot Script

Capture these screens:

- `docs/screenshots/dashboard.png`
- `docs/screenshots/stockpage.png`
- `docs/screenshots/shoppingpage.png`
- `docs/screenshots/reportspage.png`
- `docs/screenshots/aboutdialog.png`

Suggested order:

1. Main window on Dashboard.
2. StockPage with table populated.
3. ShoppingPage with suggestions.
4. ReportsPage with consolidated report.
5. AboutDialog centered over MainWindow.

## Short GIF Script

Target duration: 20-30 seconds.

1. Start on Dashboard.
2. Click StockPage and show filter/search briefly.
3. Click ReportsPage and refresh report.
4. Click LogsPage and filter ERROR.
5. Open AboutDialog.
6. End on Dashboard.
