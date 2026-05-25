cmake_minimum_required(VERSION 3.16)

project(KAGEHomeSupply VERSION 1.2.1 LANGUAGES CXX)

set(QKAGE_RELEASE_VERSION "v1.2.1-beta")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 QUIET COMPONENTS Widgets)

set(QKAGE_QT_SOURCES)
if(Qt6Widgets_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    list(APPEND QKAGE_QT_SOURCES
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/ui/theme/ThemeManager.cpp
        src/ui/theme/ThemeManager.h
        src/ui/dialogs/AddStockItemDialog.cpp
        src/ui/dialogs/AddStockItemDialog.h
        src/ui/dialogs/AboutDialog.cpp
        src/ui/dialogs/AboutDialog.h
        src/ui/dialogs/ConsumeStockItemDialog.cpp
        src/ui/dialogs/ConsumeStockItemDialog.h
        src/ui/dialogs/EditStockItemDialog.cpp
        src/ui/dialogs/EditStockItemDialog.h
        src/ui/widgets/InfoCard.cpp
        src/ui/widgets/InfoCard.h
        src/ui/widgets/FeedbackBanner.cpp
        src/ui/widgets/FeedbackBanner.h
        src/ui/widgets/OperationalChartWidget.cpp
        src/ui/widgets/OperationalChartWidget.h
        src/ui/widgets/StatusBadge.cpp
        src/ui/widgets/StatusBadge.h
        src/ui/pages/DashboardPage.cpp
        src/ui/pages/DashboardPage.h
        src/ui/pages/StockPage.cpp
        src/ui/pages/StockPage.h
        src/ui/pages/ShoppingPage.cpp
        src/ui/pages/ShoppingPage.h
        src/ui/pages/PiggyBankPage.cpp
        src/ui/pages/PiggyBankPage.h
        src/ui/pages/InvestmentsPage.cpp
        src/ui/pages/InvestmentsPage.h
        src/ui/pages/ReportsPage.cpp
        src/ui/pages/ReportsPage.h
        src/ui/pages/LogsPage.cpp
        src/ui/pages/LogsPage.h
        src/ui/pages/SettingsPage.cpp
        src/ui/pages/SettingsPage.h
        resources.qrc
    )
endif()

add_executable(kage-home-supply
    src/main.cpp
    src/cli/CommandDispatcher.cpp
    src/cli/CommandDispatcher.h
    src/controllers/ControllerResult.h
    src/controllers/StockController.cpp
    src/controllers/StockController.h
    src/controllers/FinanceController.cpp
    src/controllers/FinanceController.h
    src/controllers/ReportController.cpp
    src/controllers/ReportController.h
    src/controllers/SystemController.cpp
    src/controllers/SystemController.h
    src/services/StockService.cpp
    src/services/StockService.h
    src/services/FinanceService.cpp
    src/services/FinanceService.h
    src/services/ReportService.cpp
    src/services/ReportService.h
    src/services/SystemService.cpp
    src/services/SystemService.h
    src/core/ErrorHandler.cpp
    src/core/ErrorHandler.h
    src/core/InventoryItem.cpp
    src/core/InventoryItem.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
    src/logging/Logger.cpp
    src/logging/Logger.h
    src/analysis/StockAutonomyAnalyzer.cpp
    src/analysis/StockAutonomyAnalyzer.h
    src/analysis/PurchaseSavingsAnalyzer.cpp
    src/analysis/PurchaseSavingsAnalyzer.h
    src/analysis/InvestmentCapitalAnalyzer.cpp
    src/analysis/InvestmentCapitalAnalyzer.h
    src/analysis/CompoundInterestProjection.cpp
    src/analysis/CompoundInterestProjection.h
    src/csv/CsvInventoryReader.cpp
    src/csv/CsvInventoryReader.h
    src/csv/CsvInventoryWriter.cpp
    src/csv/CsvInventoryWriter.h
    src/csv/CsvInventoryFormat.h
    src/storage/PiggyBankManager.cpp
    src/storage/PiggyBankManager.h
    src/storage/StockConsumptionManager.cpp
    src/storage/StockConsumptionManager.h
    src/report/MonthlyReportGenerator.cpp
    src/report/MonthlyReportGenerator.h
    src/report/MarkdownReportExporter.cpp
    src/report/MarkdownReportExporter.h
    src/backup/BackupManager.cpp
    src/backup/BackupManager.h
    src/security/DataIntegrityChecker.cpp
    src/security/DataIntegrityChecker.h
    src/history/StockMovementHistory.cpp
    src/history/StockMovementHistory.h
    src/planning/ShoppingListGenerator.cpp
    src/planning/ShoppingListGenerator.h
    src/planning/ExpirationAnalyzer.cpp
    src/planning/ExpirationAnalyzer.h
    src/planning/StockRotationAdvisor.cpp
    src/planning/StockRotationAdvisor.h
    ${QKAGE_QT_SOURCES}
)
set_target_properties(kage-home-supply PROPERTIES OUTPUT_NAME qKAGE_HOME_SUPPLY)

if(Qt6Widgets_FOUND)
    target_link_libraries(kage-home-supply PRIVATE Qt6::Widgets)
    target_compile_definitions(kage-home-supply PRIVATE QKAGE_ENABLE_QT_UI)
    message(STATUS "Qt6 Widgets encontrado; bootstrap Qt habilitado neste build.")
else()
    message(STATUS "Qt6 Widgets nao encontrado; bootstrap Qt sera ignorado neste build.")
endif()

add_executable(error-handler-tests
    tests/error_handler_tests.cpp
    src/core/ErrorHandler.cpp
    src/core/ErrorHandler.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
    src/logging/Logger.cpp
    src/logging/Logger.h
)

target_include_directories(error-handler-tests PRIVATE src)

add_executable(service-layer-tests
    tests/service_layer_tests.cpp
    src/cli/CommandDispatcher.cpp
    src/cli/CommandDispatcher.h
    src/controllers/ControllerResult.h
    src/controllers/StockController.cpp
    src/controllers/StockController.h
    src/controllers/FinanceController.cpp
    src/controllers/FinanceController.h
    src/controllers/ReportController.cpp
    src/controllers/ReportController.h
    src/controllers/SystemController.cpp
    src/controllers/SystemController.h
    src/services/StockService.cpp
    src/services/StockService.h
    src/services/FinanceService.cpp
    src/services/FinanceService.h
    src/services/ReportService.cpp
    src/services/ReportService.h
    src/services/SystemService.cpp
    src/services/SystemService.h
    src/core/ErrorHandler.cpp
    src/core/ErrorHandler.h
    src/core/InventoryItem.cpp
    src/core/InventoryItem.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
    src/logging/Logger.cpp
    src/logging/Logger.h
    src/analysis/StockAutonomyAnalyzer.cpp
    src/analysis/StockAutonomyAnalyzer.h
    src/analysis/PurchaseSavingsAnalyzer.cpp
    src/analysis/PurchaseSavingsAnalyzer.h
    src/analysis/InvestmentCapitalAnalyzer.cpp
    src/analysis/InvestmentCapitalAnalyzer.h
    src/analysis/CompoundInterestProjection.cpp
    src/analysis/CompoundInterestProjection.h
    src/csv/CsvInventoryReader.cpp
    src/csv/CsvInventoryReader.h
    src/csv/CsvInventoryWriter.cpp
    src/csv/CsvInventoryWriter.h
    src/csv/CsvInventoryFormat.h
    src/storage/PiggyBankManager.cpp
    src/storage/PiggyBankManager.h
    src/storage/StockConsumptionManager.cpp
    src/storage/StockConsumptionManager.h
    src/report/MonthlyReportGenerator.cpp
    src/report/MonthlyReportGenerator.h
    src/report/MarkdownReportExporter.cpp
    src/report/MarkdownReportExporter.h
    src/backup/BackupManager.cpp
    src/backup/BackupManager.h
    src/security/DataIntegrityChecker.cpp
    src/security/DataIntegrityChecker.h
    src/history/StockMovementHistory.cpp
    src/history/StockMovementHistory.h
    src/planning/ShoppingListGenerator.cpp
    src/planning/ShoppingListGenerator.h
    src/planning/ExpirationAnalyzer.cpp
    src/planning/ExpirationAnalyzer.h
    src/planning/StockRotationAdvisor.cpp
    src/planning/StockRotationAdvisor.h
)

target_include_directories(service-layer-tests PRIVATE src)

add_executable(controller-tests
    tests/controller_tests.cpp
    src/cli/CommandDispatcher.cpp
    src/cli/CommandDispatcher.h
    src/controllers/ControllerResult.h
    src/controllers/StockController.cpp
    src/controllers/StockController.h
    src/controllers/FinanceController.cpp
    src/controllers/FinanceController.h
    src/controllers/ReportController.cpp
    src/controllers/ReportController.h
    src/controllers/SystemController.cpp
    src/controllers/SystemController.h
    src/services/StockService.cpp
    src/services/StockService.h
    src/services/FinanceService.cpp
    src/services/FinanceService.h
    src/services/ReportService.cpp
    src/services/ReportService.h
    src/services/SystemService.cpp
    src/services/SystemService.h
    src/core/ErrorHandler.cpp
    src/core/ErrorHandler.h
    src/core/InventoryItem.cpp
    src/core/InventoryItem.h
    src/config/ConfigManager.cpp
    src/config/ConfigManager.h
    src/logging/Logger.cpp
    src/logging/Logger.h
    src/analysis/StockAutonomyAnalyzer.cpp
    src/analysis/StockAutonomyAnalyzer.h
    src/analysis/PurchaseSavingsAnalyzer.cpp
    src/analysis/PurchaseSavingsAnalyzer.h
    src/analysis/InvestmentCapitalAnalyzer.cpp
    src/analysis/InvestmentCapitalAnalyzer.h
    src/analysis/CompoundInterestProjection.cpp
    src/analysis/CompoundInterestProjection.h
    src/csv/CsvInventoryReader.cpp
    src/csv/CsvInventoryReader.h
    src/csv/CsvInventoryWriter.cpp
    src/csv/CsvInventoryWriter.h
    src/csv/CsvInventoryFormat.h
    src/storage/PiggyBankManager.cpp
    src/storage/PiggyBankManager.h
    src/storage/StockConsumptionManager.cpp
    src/storage/StockConsumptionManager.h
    src/report/MonthlyReportGenerator.cpp
    src/report/MonthlyReportGenerator.h
    src/report/MarkdownReportExporter.cpp
    src/report/MarkdownReportExporter.h
    src/backup/BackupManager.cpp
    src/backup/BackupManager.h
    src/security/DataIntegrityChecker.cpp
    src/security/DataIntegrityChecker.h
    src/history/StockMovementHistory.cpp
    src/history/StockMovementHistory.h
    src/planning/ShoppingListGenerator.cpp
    src/planning/ShoppingListGenerator.h
    src/planning/ExpirationAnalyzer.cpp
    src/planning/ExpirationAnalyzer.h
    src/planning/StockRotationAdvisor.cpp
    src/planning/StockRotationAdvisor.h
)

target_include_directories(controller-tests PRIVATE src)

add_test(
    NAME csv_module_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_csv_tests.sh $<TARGET_FILE:kage-home-supply>
)

add_test(
    NAME error_handler_tests
    COMMAND $<TARGET_FILE:error-handler-tests>
)

add_test(
    NAME service_layer_tests
    COMMAND $<TARGET_FILE:service-layer-tests>
)

add_test(
    NAME controller_tests
    COMMAND $<TARGET_FILE:controller-tests>
)

add_test(
    NAME cli_help_preserved
    COMMAND $<TARGET_FILE:kage-home-supply> help
)
set_tests_properties(
    cli_help_preserved
    PROPERTIES PASS_REGULAR_EXPRESSION "KAGE Home Supply - Ajuda da CLI"
)

add_test(
    NAME cli_shopping_list_preserved
    COMMAND $<TARGET_FILE:kage-home-supply> shopping-list ${CMAKE_CURRENT_SOURCE_DIR}/data/estoque.csv
)
set_tests_properties(
    cli_shopping_list_preserved
    PROPERTIES PASS_REGULAR_EXPRESSION "KAGE Home Supply - Lista de compras"
)

add_test(
    NAME cli_backup_preserved
    COMMAND $<TARGET_FILE:kage-home-supply> backup ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/test-backups
)
set_tests_properties(
    cli_backup_preserved
    PROPERTIES PASS_REGULAR_EXPRESSION "Relatorio de backup"
)

if(Qt6Widgets_FOUND)
    add_executable(qt-widget-tests
        tests/qt_widget_tests.cpp
        src/controllers/ControllerResult.h
        src/controllers/StockController.cpp
        src/controllers/StockController.h
        src/controllers/FinanceController.cpp
        src/controllers/FinanceController.h
        src/controllers/ReportController.cpp
        src/controllers/ReportController.h
        src/controllers/SystemController.cpp
        src/controllers/SystemController.h
        src/services/StockService.cpp
        src/services/StockService.h
        src/services/FinanceService.cpp
        src/services/FinanceService.h
        src/services/ReportService.cpp
        src/services/ReportService.h
        src/services/SystemService.cpp
        src/services/SystemService.h
        src/core/ErrorHandler.cpp
        src/core/ErrorHandler.h
        src/core/InventoryItem.cpp
        src/core/InventoryItem.h
        src/config/ConfigManager.cpp
        src/config/ConfigManager.h
        src/logging/Logger.cpp
        src/logging/Logger.h
        src/analysis/StockAutonomyAnalyzer.cpp
        src/analysis/StockAutonomyAnalyzer.h
        src/analysis/PurchaseSavingsAnalyzer.cpp
        src/analysis/PurchaseSavingsAnalyzer.h
        src/analysis/InvestmentCapitalAnalyzer.cpp
        src/analysis/InvestmentCapitalAnalyzer.h
        src/analysis/CompoundInterestProjection.cpp
        src/analysis/CompoundInterestProjection.h
        src/csv/CsvInventoryReader.cpp
        src/csv/CsvInventoryReader.h
        src/csv/CsvInventoryWriter.cpp
        src/csv/CsvInventoryWriter.h
        src/csv/CsvInventoryFormat.h
        src/storage/PiggyBankManager.cpp
        src/storage/PiggyBankManager.h
        src/storage/StockConsumptionManager.cpp
        src/storage/StockConsumptionManager.h
        src/report/MonthlyReportGenerator.cpp
        src/report/MonthlyReportGenerator.h
        src/report/MarkdownReportExporter.cpp
        src/report/MarkdownReportExporter.h
        src/backup/BackupManager.cpp
        src/backup/BackupManager.h
        src/security/DataIntegrityChecker.cpp
        src/security/DataIntegrityChecker.h
        src/history/StockMovementHistory.cpp
        src/history/StockMovementHistory.h
        src/planning/ShoppingListGenerator.cpp
        src/planning/ShoppingListGenerator.h
        src/planning/ExpirationAnalyzer.cpp
        src/planning/ExpirationAnalyzer.h
        src/planning/StockRotationAdvisor.cpp
        src/planning/StockRotationAdvisor.h
        ${QKAGE_QT_SOURCES}
    )
    target_include_directories(qt-widget-tests PRIVATE src)
    target_link_libraries(qt-widget-tests PRIVATE Qt6::Widgets)

    add_test(
        NAME qt_widgets_build_tests
        COMMAND $<TARGET_FILE:qt-widget-tests>
    )
    set_tests_properties(qt_widgets_build_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_test(
        NAME qt_gui_bootstrap_opens
        COMMAND $<TARGET_FILE:kage-home-supply> --gui
    )
    set_tests_properties(
        qt_gui_bootstrap_opens
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QKAGE_QT_EXIT_AFTER_MS=50"
    )
else()
    add_test(
        NAME qt_widgets_build_tests
        COMMAND ${CMAKE_COMMAND} -E echo "Qt6 Widgets nao encontrado; teste de build Qt marcado como skip operacional"
    )
endif()
