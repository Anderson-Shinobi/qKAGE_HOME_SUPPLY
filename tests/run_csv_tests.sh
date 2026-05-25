#!/usr/bin/env bash
#
# KAGE Home Supply - Testes simples do modulo CSV

set -Eeuo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="${1:-${PROJECT_ROOT}/build/kage-home-supply}"
TEST_DIR="$(mktemp -d /tmp/kage_csv_tests_XXXXXX)"
CSV_FILE="${TEST_DIR}/estoque.csv"

fail() {
  echo "Falha: $*" >&2
  exit 1
}

assert_contains() {
  local content="$1"
  local expected="$2"
  [[ "${content}" == *"${expected}"* ]] || fail "esperado encontrar: ${expected}"
}

[[ -x "${BINARY}" ]] || fail "binario nao encontrado ou sem permissao de execucao: ${BINARY}"

echo "[1/93] adiciona item valido e preserva cabecalho"
output="$("${BINARY}" add "Arroz" "Alimentos" 5 "kg" 1 7.5 2027-12-31 1 "${CSV_FILE}")"
assert_contains "${output}" "Item adicionado ao estoque: Arroz"
header="$(head -n 1 "${CSV_FILE}")"
[[ "${header}" == "item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo" ]] || fail "cabecalho CSV foi alterado"
line_count="$(wc -l < "${CSV_FILE}")"
[[ "${line_count}" -eq 2 ]] || fail "CSV deveria conter cabecalho e 1 item"

echo "[2/93] impede cadastro com item vazio"
if "${BINARY}" add "" "Alimentos" 1 "kg" 1 2.5 2027-12-31 1 "${CSV_FILE}" >/tmp/kage_csv_empty.out 2>&1; then
  fail "cadastro vazio deveria falhar"
fi
assert_contains "$(cat /tmp/kage_csv_empty.out)" "item e obrigatorio"

echo "[3/93] impede quantidade negativa"
if "${BINARY}" add "Feijao" "Alimentos" -1 "kg" 1 8.5 2027-12-31 1 "${CSV_FILE}" >/tmp/kage_csv_negative.out 2>&1; then
  fail "quantidade negativa deveria falhar"
fi
assert_contains "$(cat /tmp/kage_csv_negative.out)" "quantidade nao pode ser negativa"

echo "[4/93] classifica item OK"
OK_CSV="${TEST_DIR}/ok.csv"
"${BINARY}" add "Arroz" "Alimentos" 4 "kg" 2 7.5 2027-12-31 1 "${OK_CSV}" >/dev/null
ok_output="$("${BINARY}" analyze "${OK_CSV}")"
assert_contains "${ok_output}" "Arroz"
assert_contains "${ok_output}" "OK"

echo "[5/93] classifica item ATENÇÃO"
ATTENTION_CSV="${TEST_DIR}/attention.csv"
"${BINARY}" add "Cafe" "Alimentos" 3 "pacote" 2 15 2027-12-31 1 "${ATTENTION_CSV}" >/dev/null
attention_output="$("${BINARY}" analyze "${ATTENTION_CSV}")"
assert_contains "${attention_output}" "Cafe"
assert_contains "${attention_output}" "ATENÇÃO"

echo "[6/93] classifica item CRÍTICO"
CRITICAL_CSV="${TEST_DIR}/critical.csv"
"${BINARY}" add "Sabao" "Limpeza" 0.5 "litro" 1 9.9 2027-12-31 1 "${CRITICAL_CSV}" >/dev/null
critical_output="$("${BINARY}" analyze "${CRITICAL_CSV}")"
assert_contains "${critical_output}" "Sabao"
assert_contains "${critical_output}" "CRÍTICO"

echo "[7/93] rejeita consumo_mensal invalido"
INVALID_CSV="${TEST_DIR}/invalid_consumption.csv"
"${BINARY}" add "Leite" "Alimentos" 2 "litro" 0 6.5 2027-12-31 1 "${INVALID_CSV}" >/dev/null
if "${BINARY}" analyze "${INVALID_CSV}" >/tmp/kage_csv_invalid_consumption.out 2>&1; then
  fail "analise com consumo_mensal invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_csv_invalid_consumption.out)" "consumo_mensal deve ser maior que zero"

echo "[8/93] calcula compra com economia"
savings_output="$("${BINARY}" savings "Creme dental" 18 8.00 5.90)"
assert_contains "${savings_output}" "Creme dental"
assert_contains "${savings_output}" "Economia unitaria: R$ 2.10"
assert_contains "${savings_output}" "Economia total: R$ 37.80"
assert_contains "${savings_output}" "Status: ECONOMIA"

echo "[9/93] calcula compra sem ganho"
no_gain_output="$("${BINARY}" savings "Sabonete" 10 3.50 3.50)"
assert_contains "${no_gain_output}" "Economia unitaria: R$ 0.00"
assert_contains "${no_gain_output}" "Economia total: R$ 0.00"
assert_contains "${no_gain_output}" "Status: SEM GANHO"

echo "[10/93] emite alerta quando atacado e maior que local"
alert_output="$("${BINARY}" savings "Detergente" 12 2.50 2.90)"
assert_contains "${alert_output}" "Economia unitaria: R$ -0.40"
assert_contains "${alert_output}" "Economia total: R$ -4.80"
assert_contains "${alert_output}" "Status: ALERTA"

echo "[11/93] rejeita quantidade invalida em economia"
if "${BINARY}" savings "Creme dental" 0 8.00 5.90 >/tmp/kage_savings_invalid_quantity.out 2>&1; then
  fail "analise de economia com quantidade invalida deveria falhar"
fi
assert_contains "$(cat /tmp/kage_savings_invalid_quantity.out)" "quantidade deve ser maior que zero"

echo "[12/93] rejeita preco invalido em economia"
if "${BINARY}" savings "Creme dental" 18 0 5.90 >/tmp/kage_savings_invalid_price.out 2>&1; then
  fail "analise de economia com preco invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_savings_invalid_price.out)" "preco local deve ser maior que zero"

echo "[13/93] calcula capital sem aporte inicial"
invest_output="$("${BINARY}" invest 40.60 12)"
assert_contains "${invest_output}" "Economia mensal: R$ 40.60"
assert_contains "${invest_output}" "Periodo em meses: 12.00"
assert_contains "${invest_output}" "Aporte inicial: R$ 0.00"
assert_contains "${invest_output}" "Capital total liberado: R$ 487.20"

echo "[14/93] calcula capital com aporte inicial"
invest_with_initial_output="$("${BINARY}" invest 40.60 12 100)"
assert_contains "${invest_with_initial_output}" "Aporte inicial: R$ 100.00"
assert_contains "${invest_with_initial_output}" "Capital total liberado: R$ 587.20"

echo "[15/93] aceita economia mensal zero"
zero_savings_output="$("${BINARY}" invest 0 12 50)"
assert_contains "${zero_savings_output}" "Economia mensal: R$ 0.00"
assert_contains "${zero_savings_output}" "Capital total liberado: R$ 50.00"

echo "[16/93] rejeita meses invalido"
if "${BINARY}" invest 40.60 0 0 >/tmp/kage_invest_invalid_months.out 2>&1; then
  fail "analise de investimento com meses invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_invest_invalid_months.out)" "meses deve ser maior que zero"

echo "[17/93] rejeita aporte inicial invalido"
if "${BINARY}" invest 40.60 12 -1 >/tmp/kage_invest_invalid_initial.out 2>&1; then
  fail "analise de investimento com aporte inicial invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_invest_invalid_initial.out)" "aporte inicial deve ser maior ou igual a zero"

echo "[18/93] cria cofrinho valido"
PIGGY_CSV="${TEST_DIR}/piggybanks.csv"
piggy_output="$("${BINARY}" piggy-add "Gas" 45.00 120.00 15.00 "${PIGGY_CSV}")"
assert_contains "${piggy_output}" "Cofrinho adicionado: Gas"
piggy_header="$(head -n 1 "${PIGGY_CSV}")"
[[ "${piggy_header}" == "nome,valor_atual,meta,aporte_mensal,status" ]] || fail "cabecalho de cofrinhos foi alterado"

echo "[19/93] rejeita cofrinho com nome vazio"
if "${BINARY}" piggy-add "" 45.00 120.00 15.00 "${PIGGY_CSV}" >/tmp/kage_piggy_empty_name.out 2>&1; then
  fail "cofrinho com nome vazio deveria falhar"
fi
assert_contains "$(cat /tmp/kage_piggy_empty_name.out)" "nome e obrigatorio"

echo "[20/93] rejeita cofrinho com meta invalida"
if "${BINARY}" piggy-add "Gas" 45.00 0 15.00 "${PIGGY_CSV}" >/tmp/kage_piggy_invalid_goal.out 2>&1; then
  fail "cofrinho com meta invalida deveria falhar"
fi
assert_contains "$(cat /tmp/kage_piggy_invalid_goal.out)" "meta deve ser maior que zero"

echo "[21/93] rejeita cofrinho com aporte invalido"
if "${BINARY}" piggy-add "Gas" 45.00 120.00 -1 "${PIGGY_CSV}" >/tmp/kage_piggy_invalid_contribution.out 2>&1; then
  fail "cofrinho com aporte invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_piggy_invalid_contribution.out)" "aporte_mensal deve ser maior ou igual a zero"

echo "[22/93] classifica cofrinho CRÍTICO"
CRITICAL_PIGGY_CSV="${TEST_DIR}/piggy_critical.csv"
"${BINARY}" piggy-add "Gas" 20.00 100.00 15.00 "${CRITICAL_PIGGY_CSV}" >/dev/null
critical_piggy_report="$("${BINARY}" piggy-report "${CRITICAL_PIGGY_CSV}")"
assert_contains "${critical_piggy_report}" "Gas"
assert_contains "${critical_piggy_report}" "20.00%"
assert_contains "${critical_piggy_report}" "CRÍTICO"

echo "[23/93] classifica cofrinho ATENÇÃO"
ATTENTION_PIGGY_CSV="${TEST_DIR}/piggy_attention.csv"
"${BINARY}" piggy-add "Gas" 50.00 100.00 15.00 "${ATTENTION_PIGGY_CSV}" >/dev/null
attention_piggy_report="$("${BINARY}" piggy-report "${ATTENTION_PIGGY_CSV}")"
assert_contains "${attention_piggy_report}" "50.00%"
assert_contains "${attention_piggy_report}" "ATENÇÃO"

echo "[24/93] classifica cofrinho OK"
OK_PIGGY_CSV="${TEST_DIR}/piggy_ok.csv"
"${BINARY}" piggy-add "Gas" 80.00 100.00 15.00 "${OK_PIGGY_CSV}" >/dev/null
ok_piggy_report="$("${BINARY}" piggy-report "${OK_PIGGY_CSV}")"
assert_contains "${ok_piggy_report}" "80.00%"
assert_contains "${ok_piggy_report}" "OK"

echo "[25/93] calcula projecao valida com juros compostos"
compound_output="$("${BINARY}" compound 40.60 0.008 120 0)"
assert_contains "${compound_output}" "Aporte mensal: R$ 40.60"
assert_contains "${compound_output}" "Taxa mensal: 0.0080"
assert_contains "${compound_output}" "Total aportado: R$ 4872.00"
assert_contains "${compound_output}" "Valor final estimado: R$ 8128.83"

echo "[26/93] calcula projecao sem aporte inicial"
compound_without_initial_output="$("${BINARY}" compound 100 0.01 2)"
assert_contains "${compound_without_initial_output}" "Aporte inicial: R$ 0.00"
assert_contains "${compound_without_initial_output}" "Total aportado: R$ 200.00"
assert_contains "${compound_without_initial_output}" "Valor final estimado: R$ 201.00"

echo "[27/93] calcula projecao com taxa zero"
compound_zero_rate_output="$("${BINARY}" compound 100 0 3 50)"
assert_contains "${compound_zero_rate_output}" "Total aportado: R$ 350.00"
assert_contains "${compound_zero_rate_output}" "Juros acumulados: R$ 0.00"
assert_contains "${compound_zero_rate_output}" "Valor final estimado: R$ 350.00"

echo "[28/93] rejeita meses invalido em juros compostos"
if "${BINARY}" compound 40.60 0.008 0 0 >/tmp/kage_compound_invalid_months.out 2>&1; then
  fail "projecao com meses invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_compound_invalid_months.out)" "meses deve ser maior que zero"

echo "[29/93] rejeita aporte negativo em juros compostos"
if "${BINARY}" compound -1 0.008 120 0 >/tmp/kage_compound_negative_contribution.out 2>&1; then
  fail "projecao com aporte mensal negativo deveria falhar"
fi
assert_contains "$(cat /tmp/kage_compound_negative_contribution.out)" "aporte mensal deve ser maior ou igual a zero"

echo "[30/93] gera relatorio mensal valido"
MONTHLY_STOCK_CSV="${TEST_DIR}/monthly_stock.csv"
MONTHLY_PIGGY_CSV="${TEST_DIR}/monthly_piggy.csv"
"${BINARY}" add "Sabao" "Limpeza" 0.5 "litro" 1 9.9 2027-12-31 1 "${MONTHLY_STOCK_CSV}" >/dev/null
"${BINARY}" add "Cafe" "Alimentos" 3 "pacote" 2 15 2027-12-31 1 "${MONTHLY_STOCK_CSV}" >/dev/null
"${BINARY}" add "Arroz" "Alimentos" 4 "kg" 2 7.5 2027-12-31 1 "${MONTHLY_STOCK_CSV}" >/dev/null
"${BINARY}" piggy-add "Gas" 50 100 15 "${MONTHLY_PIGGY_CSV}" >/dev/null
"${BINARY}" piggy-add "Reserva" 100 100 20 "${MONTHLY_PIGGY_CSV}" >/dev/null
monthly_report="$("${BINARY}" monthly-report "${MONTHLY_STOCK_CSV}" "${MONTHLY_PIGGY_CSV}")"
assert_contains "${monthly_report}" "[ESTOQUE]"
assert_contains "${monthly_report}" "Total de itens cadastrados: 3"
assert_contains "${monthly_report}" "Itens CRÍTICO: 1"
assert_contains "${monthly_report}" "Itens ATENÇÃO: 1"
assert_contains "${monthly_report}" "Itens OK: 1"
assert_contains "${monthly_report}" "Cofrinhos cadastrados: 2"
assert_contains "${monthly_report}" "Percentual medio das metas: 75.00%"

echo "[31/93] gera relatorio mensal com estoque vazio"
EMPTY_STOCK_CSV="${TEST_DIR}/monthly_empty_stock.csv"
EMPTY_PIGGY_CSV="${TEST_DIR}/monthly_empty_stock_piggy.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${EMPTY_STOCK_CSV}"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${EMPTY_PIGGY_CSV}"
empty_stock_report="$("${BINARY}" monthly-report "${EMPTY_STOCK_CSV}" "${EMPTY_PIGGY_CSV}")"
assert_contains "${empty_stock_report}" "Total de itens cadastrados: 0"
assert_contains "${empty_stock_report}" "Estoque monitorado: sem itens cadastrados"

echo "[32/93] gera relatorio mensal com cofrinhos vazios"
EMPTY_PIGGY_ONLY_CSV="${TEST_DIR}/monthly_empty_piggy.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${EMPTY_PIGGY_ONLY_CSV}"
empty_piggy_report="$("${BINARY}" monthly-report "${MONTHLY_STOCK_CSV}" "${EMPTY_PIGGY_ONLY_CSV}")"
assert_contains "${empty_piggy_report}" "Cofrinhos cadastrados: 0"
assert_contains "${empty_piggy_report}" "Status: nenhum cofrinho cadastrado"

echo "[33/93] destaca item critico no relatorio mensal"
CRITICAL_MONTHLY_STOCK_CSV="${TEST_DIR}/monthly_critical_stock.csv"
"${BINARY}" add "Detergente" "Limpeza" 0.2 "litro" 1 3.5 2027-12-31 1 "${CRITICAL_MONTHLY_STOCK_CSV}" >/dev/null
critical_monthly_report="$("${BINARY}" monthly-report "${CRITICAL_MONTHLY_STOCK_CSV}" "${EMPTY_PIGGY_ONLY_CSV}")"
assert_contains "${critical_monthly_report}" "Itens CRÍTICO: 1"

echo "[34/93] mantem economia zero no relatorio mensal sem historico de compras"
assert_contains "${monthly_report}" "Economia total estimada: R$ 0.00"
assert_contains "${monthly_report}" "Capital liberado para investimento: R$ 0.00"

echo "[35/93] exporta relatorio Markdown valido"
REPORTS_DIR="${TEST_DIR}/reports"
export_output="$("${BINARY}" export-report "${MONTHLY_STOCK_CSV}" "${MONTHLY_PIGGY_CSV}" "${REPORTS_DIR}")"
assert_contains "${export_output}" "Status da exportacao: Exportacao Markdown concluida."
assert_contains "${export_output}" "Arquivo gerado:"
generated_report="$(find "${REPORTS_DIR}" -maxdepth 1 -type f -name 'monthly_report_*.md' | head -n 1)"
[[ -f "${generated_report}" ]] || fail "arquivo Markdown nao foi gerado"

echo "[36/93] cria automaticamente diretorio reports"
AUTO_REPORTS_DIR="${TEST_DIR}/auto_reports"
[[ ! -e "${AUTO_REPORTS_DIR}" ]] || fail "diretorio de teste ja existia"
"${BINARY}" export-report "${MONTHLY_STOCK_CSV}" "${MONTHLY_PIGGY_CSV}" "${AUTO_REPORTS_DIR}" >/dev/null
[[ -d "${AUTO_REPORTS_DIR}" ]] || fail "diretorio reports nao foi criado automaticamente"

echo "[37/93] gera Markdown com conteudo esperado"
markdown_content="$(cat "${generated_report}")"
assert_contains "${markdown_content}" "# KAGE Home Supply"
assert_contains "${markdown_content}" "Data de geracao:"
assert_contains "${markdown_content}" "## ESTOQUE"
assert_contains "${markdown_content}" "## ECONOMIA"
assert_contains "${markdown_content}" "## COFRINHOS"
assert_contains "${markdown_content}" "## INVESTIMENTO"
assert_contains "${markdown_content}" "## RESUMO FINAL"

echo "[38/93] falha de forma controlada em caminho invalido"
INVALID_REPORTS_PATH="${TEST_DIR}/not_a_directory"
printf 'bloqueio\n' > "${INVALID_REPORTS_PATH}"
if "${BINARY}" export-report "${MONTHLY_STOCK_CSV}" "${MONTHLY_PIGGY_CSV}" "${INVALID_REPORTS_PATH}" >/tmp/kage_export_invalid_path.out 2>&1; then
  fail "exportacao em caminho invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_export_invalid_path.out)" "caminho de relatorios nao e um diretorio"

echo "[39/93] cria backup valido"
BACKUP_SOURCE="${TEST_DIR}/backup_source"
BACKUP_ROOT="${TEST_DIR}/backup_root"
mkdir -p "${BACKUP_SOURCE}/data" "${BACKUP_SOURCE}/reports"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${BACKUP_SOURCE}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${BACKUP_SOURCE}/data/piggybanks.csv"
printf '# Relatorio\n' > "${BACKUP_SOURCE}/reports/monthly_report_test.md"
backup_output="$("${BINARY}" backup "${BACKUP_SOURCE}" "${BACKUP_ROOT}")"
assert_contains "${backup_output}" "Status final: Backup concluido."
assert_contains "${backup_output}" "Arquivos copiados: 3"
backup_dir="$(find "${BACKUP_ROOT}" -mindepth 1 -maxdepth 1 -type d -name 'backup_*' | head -n 1)"
[[ -d "${backup_dir}" ]] || fail "diretorio de backup nao foi criado"

echo "[40/93] cria automaticamente diretorio de backups"
AUTO_BACKUP_SOURCE="${TEST_DIR}/auto_backup_source"
AUTO_BACKUP_ROOT="${TEST_DIR}/auto_backup_root"
mkdir -p "${AUTO_BACKUP_SOURCE}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${AUTO_BACKUP_SOURCE}/data/estoque.csv"
[[ ! -e "${AUTO_BACKUP_ROOT}" ]] || fail "diretorio de backup ja existia"
"${BINARY}" backup "${AUTO_BACKUP_SOURCE}" "${AUTO_BACKUP_ROOT}" >/dev/null
[[ -d "${AUTO_BACKUP_ROOT}" ]] || fail "diretorio de backup nao foi criado automaticamente"

echo "[41/93] ignora reports ausente"
NO_REPORTS_SOURCE="${TEST_DIR}/no_reports_source"
NO_REPORTS_ROOT="${TEST_DIR}/no_reports_root"
mkdir -p "${NO_REPORTS_SOURCE}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${NO_REPORTS_SOURCE}/data/estoque.csv"
no_reports_output="$("${BINARY}" backup "${NO_REPORTS_SOURCE}" "${NO_REPORTS_ROOT}")"
assert_contains "${no_reports_output}" "reports/*.md (diretorio inexistente)"

echo "[42/93] ignora piggybanks ausente"
NO_PIGGY_SOURCE="${TEST_DIR}/no_piggy_source"
NO_PIGGY_ROOT="${TEST_DIR}/no_piggy_root"
mkdir -p "${NO_PIGGY_SOURCE}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${NO_PIGGY_SOURCE}/data/estoque.csv"
no_piggy_output="$("${BINARY}" backup "${NO_PIGGY_SOURCE}" "${NO_PIGGY_ROOT}")"
assert_contains "${no_piggy_output}" "piggybanks.csv (inexistente)"

echo "[43/93] preserva estrutura do backup"
[[ -f "${backup_dir}/data/estoque.csv" ]] || fail "estoque.csv nao foi preservado em data/"
[[ -f "${backup_dir}/data/piggybanks.csv" ]] || fail "piggybanks.csv nao foi preservado em data/"
[[ -f "${backup_dir}/reports/monthly_report_test.md" ]] || fail "relatorio Markdown nao foi preservado em reports/"

echo "[44/93] integridade aprova CSV valido"
INTEGRITY_VALID_ROOT="${TEST_DIR}/integrity_valid"
mkdir -p "${INTEGRITY_VALID_ROOT}/data" "${INTEGRITY_VALID_ROOT}/reports" "${INTEGRITY_VALID_ROOT}/backups"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nArroz,Alimentos,5,kg,1,7.5,2027-12-31,1\n' > "${INTEGRITY_VALID_ROOT}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\nGas,45,120,15,ATENÇÃO\n' > "${INTEGRITY_VALID_ROOT}/data/piggybanks.csv"
integrity_valid_output="$("${BINARY}" integrity-check "${INTEGRITY_VALID_ROOT}")"
assert_contains "${integrity_valid_output}" "Status final: OK"

echo "[45/93] integridade detecta CSV inexistente"
INTEGRITY_MISSING_ROOT="${TEST_DIR}/integrity_missing"
mkdir -p "${INTEGRITY_MISSING_ROOT}/data"
if "${BINARY}" integrity-check "${INTEGRITY_MISSING_ROOT}" >/tmp/kage_integrity_missing.out 2>&1; then
  fail "integridade com CSV inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_integrity_missing.out)" "CSV inexistente"
assert_contains "$(cat /tmp/kage_integrity_missing.out)" "Status final: FAILED"

echo "[46/93] integridade detecta cabecalho invalido"
INTEGRITY_BAD_HEADER_ROOT="${TEST_DIR}/integrity_bad_header"
mkdir -p "${INTEGRITY_BAD_HEADER_ROOT}/data"
printf 'bad,header\n' > "${INTEGRITY_BAD_HEADER_ROOT}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${INTEGRITY_BAD_HEADER_ROOT}/data/piggybanks.csv"
if "${BINARY}" integrity-check "${INTEGRITY_BAD_HEADER_ROOT}" >/tmp/kage_integrity_bad_header.out 2>&1; then
  fail "integridade com cabecalho invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_integrity_bad_header.out)" "cabecalho invalido"

echo "[47/93] integridade avisa sobre linha vazia"
INTEGRITY_BLANK_ROOT="${TEST_DIR}/integrity_blank"
mkdir -p "${INTEGRITY_BLANK_ROOT}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n\nArroz,Alimentos,5,kg,1,7.5,2027-12-31,1\n' > "${INTEGRITY_BLANK_ROOT}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${INTEGRITY_BLANK_ROOT}/data/piggybanks.csv"
blank_output="$("${BINARY}" integrity-check "${INTEGRITY_BLANK_ROOT}")"
assert_contains "${blank_output}" "linha vazia"
assert_contains "${blank_output}" "Status final: WARNING"

echo "[48/93] integridade detecta coluna faltando"
INTEGRITY_MISSING_COLUMN_ROOT="${TEST_DIR}/integrity_missing_column"
mkdir -p "${INTEGRITY_MISSING_COLUMN_ROOT}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nArroz,Alimentos,5\n' > "${INTEGRITY_MISSING_COLUMN_ROOT}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${INTEGRITY_MISSING_COLUMN_ROOT}/data/piggybanks.csv"
if "${BINARY}" integrity-check "${INTEGRITY_MISSING_COLUMN_ROOT}" >/tmp/kage_integrity_missing_column.out 2>&1; then
  fail "integridade com coluna faltando deveria falhar"
fi
assert_contains "$(cat /tmp/kage_integrity_missing_column.out)" "quantidade incorreta de colunas"

echo "[49/93] integridade detecta valor numerico invalido"
INTEGRITY_BAD_NUMBER_ROOT="${TEST_DIR}/integrity_bad_number"
mkdir -p "${INTEGRITY_BAD_NUMBER_ROOT}/data"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nArroz,Alimentos,abc,kg,1,7.5,2027-12-31,1\n' > "${INTEGRITY_BAD_NUMBER_ROOT}/data/estoque.csv"
printf 'nome,valor_atual,meta,aporte_mensal,status\n' > "${INTEGRITY_BAD_NUMBER_ROOT}/data/piggybanks.csv"
if "${BINARY}" integrity-check "${INTEGRITY_BAD_NUMBER_ROOT}" >/tmp/kage_integrity_bad_number.out 2>&1; then
  fail "integridade com numero invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_integrity_bad_number.out)" "valor numerico invalido"

echo "[50/93] integrity-check --fix recria CSV e faz backup automatico"
INTEGRITY_FIX_ROOT="${TEST_DIR}/integrity_fix"
mkdir -p "${INTEGRITY_FIX_ROOT}/data" "${INTEGRITY_FIX_ROOT}/reports"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n\nArroz,Alimentos,5,kg,1,7.5,2027-12-31,1\n' > "${INTEGRITY_FIX_ROOT}/data/estoque.csv"
fix_output="$("${BINARY}" integrity-check --fix "${INTEGRITY_FIX_ROOT}")"
assert_contains "${fix_output}" "CSV recriado:"
assert_contains "${fix_output}" "linhas vazias removidas:"
assert_contains "${fix_output}" "backup automatico criado:"
[[ -f "${INTEGRITY_FIX_ROOT}/data/piggybanks.csv" ]] || fail "--fix nao recriou piggybanks.csv"
if grep -q '^$' "${INTEGRITY_FIX_ROOT}/data/estoque.csv"; then
  fail "--fix nao removeu linha vazia"
fi
fix_backup_dir="$(find "${INTEGRITY_FIX_ROOT}/backups" -mindepth 1 -maxdepth 1 -type d -name 'backup_*' | head -n 1)"
[[ -d "${fix_backup_dir}" ]] || fail "--fix nao criou backup automatico"

echo "[51/93] registra movimentacao ADD"
HISTORY_ADD_CSV="${TEST_DIR}/history_add_stock.csv"
"${BINARY}" add "Acucar" "Alimentos" 2 "kg" 1 4.5 2027-12-31 1 "${HISTORY_ADD_CSV}" >/dev/null
history_add_report="$("${BINARY}" history-report "${TEST_DIR}/stock_movements.csv")"
assert_contains "${history_add_report}" "ADD"
assert_contains "${history_add_report}" "Acucar"

echo "[52/93] registra movimentacao CONSUME e atualiza estoque"
"${BINARY}" consume "Acucar" 1 "uso mensal" "${HISTORY_ADD_CSV}" >/dev/null
history_consume_report="$("${BINARY}" history-report "${TEST_DIR}/stock_movements.csv")"
assert_contains "${history_consume_report}" "CONSUME"
assert_contains "${history_consume_report}" "uso mensal"
assert_contains "$(cat "${HISTORY_ADD_CSV}")" "Acucar,Alimentos,1,kg"

echo "[53/93] valida erros do consumo"
if "${BINARY}" consume "Inexistente" 1 "teste" "${HISTORY_ADD_CSV}" >/tmp/kage_consume_missing_item.out 2>&1; then
  fail "consumo de item inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_consume_missing_item.out)" "item nao encontrado"
if "${BINARY}" consume "Acucar" 99 "teste" "${HISTORY_ADD_CSV}" >/tmp/kage_consume_insufficient.out 2>&1; then
  fail "consumo acima do estoque deveria falhar"
fi
assert_contains "$(cat /tmp/kage_consume_insufficient.out)" "estoque insuficiente"
if "${BINARY}" consume "Acucar" -1 "teste" "${HISTORY_ADD_CSV}" >/tmp/kage_consume_negative.out 2>&1; then
  fail "consumo com quantidade negativa deveria falhar"
fi
assert_contains "$(cat /tmp/kage_consume_negative.out)" "quantidade deve ser maior que zero"
if "${BINARY}" consume "Acucar" 1 "teste" "${TEST_DIR}/missing_stock.csv" >/tmp/kage_consume_missing_csv.out 2>&1; then
  fail "consumo com CSV inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_consume_missing_csv.out)" "arquivo inexistente"

echo "[54/93] valida relatorio e rejeicoes do historico"
REMOVE_CSV="${TEST_DIR}/remove_stock.csv"
"${BINARY}" add "Remover item" "Testes" 2 "un" 1 3.5 2027-12-31 1 "${REMOVE_CSV}" >/dev/null
remove_output="$("${BINARY}" remove "Remover item" "${REMOVE_CSV}")"
assert_contains "${remove_output}" "Item removido do estoque"
if grep -q "Remover item" "${REMOVE_CSV}"; then
  fail "remove nao excluiu item do CSV"
fi
remove_history_report="$("${BINARY}" history-report "${TEST_DIR}/stock_movements.csv")"
assert_contains "${remove_history_report}" "ADJUST"
assert_contains "${remove_history_report}" "Item removido do estoque"
if "${BINARY}" remove "Inexistente" "${REMOVE_CSV}" >/tmp/kage_remove_missing_item.out 2>&1; then
  fail "remocao de item inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_remove_missing_item.out)" "item nao encontrado"

INVALID_HISTORY_TYPE="${TEST_DIR}/invalid_history_type.csv"
printf 'data_hora,tipo,item,quantidade,unidade,observacao\n2026-05-23 10:00:00,BAD,Arroz,1,kg,teste\n' > "${INVALID_HISTORY_TYPE}"
if "${BINARY}" history-report "${INVALID_HISTORY_TYPE}" >/tmp/kage_history_invalid_type.out 2>&1; then
  fail "historico com tipo invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_history_invalid_type.out)" "tipo de movimentacao invalido"

echo "[55/93] rejeita item vazio no historico"
INVALID_HISTORY_ITEM="${TEST_DIR}/invalid_history_item.csv"
printf 'data_hora,tipo,item,quantidade,unidade,observacao\n2026-05-23 10:00:00,ADD,,1,kg,teste\n' > "${INVALID_HISTORY_ITEM}"
if "${BINARY}" history-report "${INVALID_HISTORY_ITEM}" >/tmp/kage_history_empty_item.out 2>&1; then
  fail "historico com item vazio deveria falhar"
fi
assert_contains "$(cat /tmp/kage_history_empty_item.out)" "item e obrigatorio"

echo "[56/93] cria historico automaticamente"
AUTO_HISTORY_CSV="${TEST_DIR}/auto_history.csv"
[[ ! -e "${AUTO_HISTORY_CSV}" ]] || fail "historico de teste ja existia"
auto_history_report="$("${BINARY}" history-report "${AUTO_HISTORY_CSV}")"
assert_contains "${auto_history_report}" "Historico de movimentacoes vazio"
[[ -f "${AUTO_HISTORY_CSV}" ]] || fail "history-report nao criou arquivo automaticamente"
assert_contains "${history_consume_report}" "data/hora"
assert_contains "${history_consume_report}" "observacao"

echo "[57/93] gera lista com item abaixo do minimo e sugestao correta"
SHOPPING_CSV="${TEST_DIR}/shopping.csv"
"${BINARY}" add "Creme dental" "Higiene" 1 "un" 2 8.0 2027-12-31 1 "${SHOPPING_CSV}" >/dev/null
"${BINARY}" add "Arroz" "Alimentos" 10 "kg" 2 7.5 2027-12-31 2 "${SHOPPING_CSV}" >/dev/null
"${BINARY}" add "Sabonete" "Higiene" 3 "un" 1 3.0 2027-12-31 3 "${SHOPPING_CSV}" >/dev/null
shopping_output="$("${BINARY}" shopping-list "${SHOPPING_CSV}")"
assert_contains "${shopping_output}" "Creme dental"
assert_contains "${shopping_output}" "5.00 un"

echo "[58/93] lista item exatamente no minimo com sugestao zero"
assert_contains "${shopping_output}" "Sabonete"
assert_contains "${shopping_output}" "0.00 un"

echo "[59/93] nao lista item acima do minimo"
if [[ "${shopping_output}" == *"Arroz"* ]]; then
  fail "item acima do minimo nao deveria aparecer na lista de compras"
fi

echo "[60/93] trata CSV vazio na lista de compras"
EMPTY_SHOPPING_CSV="${TEST_DIR}/empty_shopping.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\n' > "${EMPTY_SHOPPING_CSV}"
empty_shopping_output="$("${BINARY}" shopping-list "${EMPTY_SHOPPING_CSV}")"
assert_contains "${empty_shopping_output}" "Nenhum item precisa de compra"

echo "[61/93] rejeita valores invalidos na lista de compras"
INVALID_SHOPPING_CSV="${TEST_DIR}/invalid_shopping.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nSabao,Limpeza,-1,un,2,4.5,2027-12-31,1\n' > "${INVALID_SHOPPING_CSV}"
if "${BINARY}" shopping-list "${INVALID_SHOPPING_CSV}" >/tmp/kage_shopping_invalid.out 2>&1; then
  fail "shopping-list com quantidade negativa deveria falhar"
fi
assert_contains "$(cat /tmp/kage_shopping_invalid.out)" "quantidade negativa"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nSabao,Limpeza,1,un,0,4.5,2027-12-31,1\n' > "${INVALID_SHOPPING_CSV}"
if "${BINARY}" shopping-list "${INVALID_SHOPPING_CSV}" >/tmp/kage_shopping_invalid_consumption.out 2>&1; then
  fail "shopping-list com consumo invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_shopping_invalid_consumption.out)" "consumo_mensal invalido"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nSabao,Limpeza,1,un,2,4.5,2027-12-31,-1\n' > "${INVALID_SHOPPING_CSV}"
if "${BINARY}" shopping-list "${INVALID_SHOPPING_CSV}" >/tmp/kage_shopping_invalid_minimum.out 2>&1; then
  fail "shopping-list com estoque_minimo invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_shopping_invalid_minimum.out)" "estoque_minimo invalido"

echo "[62/93] rejeita CSV inexistente na lista de compras"
if "${BINARY}" shopping-list "${TEST_DIR}/missing_shopping.csv" >/tmp/kage_shopping_missing_csv.out 2>&1; then
  fail "shopping-list com CSV inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_shopping_missing_csv.out)" "arquivo inexistente"

echo "[63/93] classifica item vencido"
EXPIRATION_CSV="${TEST_DIR}/expiration.csv"
PAST_DATE="$(date -d 'yesterday' '+%Y-%m-%d')"
CRITICAL_DATE="$(date -d '+15 days' '+%Y-%m-%d')"
ATTENTION_DATE="$(date -d '+60 days' '+%Y-%m-%d')"
OK_DATE="$(date -d '+120 days' '+%Y-%m-%d')"
"${BINARY}" add "Iogurte" "Alimentos" 2 "un" 1 5.0 "${PAST_DATE}" 1 "${EXPIRATION_CSV}" >/dev/null
"${BINARY}" add "Leite" "Alimentos" 2 "un" 1 6.0 "${CRITICAL_DATE}" 1 "${EXPIRATION_CSV}" >/dev/null
"${BINARY}" add "Arroz" "Alimentos" 2 "kg" 1 7.5 "${ATTENTION_DATE}" 1 "${EXPIRATION_CSV}" >/dev/null
"${BINARY}" add "Sabao" "Limpeza" 2 "un" 1 4.5 "${OK_DATE}" 1 "${EXPIRATION_CSV}" >/dev/null
"${BINARY}" add "Sal" "Alimentos" 2 "kg" 1 2.5 "sem_validade" 1 "${EXPIRATION_CSV}" >/dev/null
expiration_output="$("${BINARY}" expiration-report "${EXPIRATION_CSV}")"
assert_contains "${expiration_output}" "Iogurte"
assert_contains "${expiration_output}" "VENCIDO"

echo "[64/93] classifica item critico por validade"
assert_contains "${expiration_output}" "Leite"
assert_contains "${expiration_output}" "CRÍTICO"

echo "[65/93] classifica item atencao por validade"
assert_contains "${expiration_output}" "Arroz"
assert_contains "${expiration_output}" "ATENÇÃO"

echo "[66/93] classifica item OK por validade"
assert_contains "${expiration_output}" "Sabao"
assert_contains "${expiration_output}" "OK"

echo "[67/93] classifica item sem validade"
assert_contains "${expiration_output}" "Sal"
assert_contains "${expiration_output}" "SEM_VALIDADE"

echo "[68/93] rejeita data invalida no relatorio de validade"
INVALID_EXPIRATION_CSV="${TEST_DIR}/invalid_expiration.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nQueijo,Alimentos,1,un,1,10,2027-99-99,1\n' > "${INVALID_EXPIRATION_CSV}"
if "${BINARY}" expiration-report "${INVALID_EXPIRATION_CSV}" >/tmp/kage_expiration_invalid.out 2>&1; then
  fail "expiration-report com data invalida deveria falhar"
fi
assert_contains "$(cat /tmp/kage_expiration_invalid.out)" "formato invalido de validade"

echo "[69/93] rotacao prioriza produto vencido"
ROTATION_CSV="${TEST_DIR}/rotation.csv"
ROTATION_PAST_DATE="$(date -d 'yesterday' '+%Y-%m-%d')"
ROTATION_30_DATE="$(date -d '+15 days' '+%Y-%m-%d')"
ROTATION_90_DATE="$(date -d '+60 days' '+%Y-%m-%d')"
"${BINARY}" add "Iogurte" "Alimentos" 2 "un" 1 5.0 "${ROTATION_PAST_DATE}" 1 "${ROTATION_CSV}" >/dev/null
"${BINARY}" add "Leite" "Alimentos" 2 "un" 1 6.0 "${ROTATION_30_DATE}" 1 "${ROTATION_CSV}" >/dev/null
"${BINARY}" add "Arroz" "Alimentos" 2 "kg" 1 7.5 "${ROTATION_90_DATE}" 1 "${ROTATION_CSV}" >/dev/null
"${BINARY}" add "Sal" "Alimentos" 2 "kg" 1 2.5 "sem_validade" 1 "${ROTATION_CSV}" >/dev/null
"${BINARY}" add "Cafe" "Alimentos" 0.5 "pacote" 1 15.0 "sem_validade" 1 "${ROTATION_CSV}" >/dev/null
rotation_output="$("${BINARY}" rotation-advice "${ROTATION_CSV}")"
assert_contains "${rotation_output}" "Iogurte"
assert_contains "${rotation_output}" "DESCARTAR"

echo "[70/93] rotacao classifica vencimento em ate 30 dias"
assert_contains "${rotation_output}" "Leite"
assert_contains "${rotation_output}" "CONSUMIR AGORA"

echo "[71/93] rotacao classifica vencimento entre 31 e 90 dias"
assert_contains "${rotation_output}" "Arroz"
assert_contains "${rotation_output}" "CONSUMIR PRIMEIRO"

echo "[72/93] rotacao trata item sem validade fora de alerta de vencimento"
assert_contains "${rotation_output}" "Sal"
assert_contains "${rotation_output}" "ESTÁVEL"

echo "[73/93] rotacao monitora baixa autonomia sem validade"
assert_contains "${rotation_output}" "Cafe"
assert_contains "${rotation_output}" "MONITORAR"

echo "[74/93] rotacao marca data invalida como dado invalido"
INVALID_ROTATION_CSV="${TEST_DIR}/invalid_rotation_date.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nQueijo,Alimentos,1,un,1,10,2027-99-99,1\n' > "${INVALID_ROTATION_CSV}"
invalid_rotation_output="$("${BINARY}" rotation-advice "${INVALID_ROTATION_CSV}")"
assert_contains "${invalid_rotation_output}" "Queijo"
assert_contains "${invalid_rotation_output}" "DADO_INVÁLIDO"

echo "[75/93] rotacao rejeita quantidade negativa"
NEGATIVE_ROTATION_CSV="${TEST_DIR}/negative_rotation.csv"
printf 'item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo\nSabao,Limpeza,-1,un,1,4.5,sem_validade,1\n' > "${NEGATIVE_ROTATION_CSV}"
if "${BINARY}" rotation-advice "${NEGATIVE_ROTATION_CSV}" >/tmp/kage_rotation_negative.out 2>&1; then
  fail "rotation-advice com quantidade negativa deveria falhar"
fi
assert_contains "$(cat /tmp/kage_rotation_negative.out)" "quantidade negativa"

echo "[76/93] cria config automaticamente"
AUTO_CONFIG_INI="${TEST_DIR}/config_auto.ini"
[[ ! -e "${AUTO_CONFIG_INI}" ]] || fail "config de teste ja existia"
auto_config_output="$("${BINARY}" config-show "${AUTO_CONFIG_INI}")"
[[ -f "${AUTO_CONFIG_INI}" ]] || fail "config-show nao criou config.ini automaticamente"
assert_contains "${auto_config_output}" "version=0.1.0"
assert_contains "${auto_config_output}" "currency=BRL"
assert_contains "${auto_config_output}" "logs_enabled=true"

echo "[77/93] le config valida"
VALID_CONFIG_INI="${TEST_DIR}/config_valid.ini"
cat > "${VALID_CONFIG_INI}" <<'CONFIG_EOF'
[system]
version=0.1.0
currency=BRL
logs_enabled=true
backup_enabled=true
reports_enabled=false
debug_mode=false

[paths]
data_dir=data
backup_dir=backups
reports_dir=reports

[limits]
critical_autonomy_months=1
warning_autonomy_months=2
expiration_critical_days=30
expiration_warning_days=90
CONFIG_EOF
valid_config_output="$("${BINARY}" config-show "${VALID_CONFIG_INI}")"
assert_contains "${valid_config_output}" "reports_enabled=false"
assert_contains "${valid_config_output}" "expiration_warning_days=90"

echo "[78/93] rejeita chave ausente em config"
MISSING_KEY_CONFIG_INI="${TEST_DIR}/config_missing_key.ini"
cat > "${MISSING_KEY_CONFIG_INI}" <<'CONFIG_EOF'
[system]
version=0.1.0
currency=BRL
logs_enabled=true
backup_enabled=true
reports_enabled=true
debug_mode=false

[paths]
data_dir=data
backup_dir=backups
reports_dir=reports

[limits]
critical_autonomy_months=1
warning_autonomy_months=2
expiration_critical_days=30
CONFIG_EOF
if "${BINARY}" config-show "${MISSING_KEY_CONFIG_INI}" >/tmp/kage_config_missing_key.out 2>&1; then
  fail "config com chave ausente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_config_missing_key.out)" "chave ausente"

echo "[79/93] rejeita valor invalido em config"
INVALID_VALUE_CONFIG_INI="${TEST_DIR}/config_invalid_value.ini"
cat > "${INVALID_VALUE_CONFIG_INI}" <<'CONFIG_EOF'
[system]
version=0.1.0
currency=BRL
logs_enabled=true
backup_enabled=yes
reports_enabled=true
debug_mode=false

[paths]
data_dir=data
backup_dir=backups
reports_dir=reports

[limits]
critical_autonomy_months=1
warning_autonomy_months=2
expiration_critical_days=30
expiration_warning_days=90
CONFIG_EOF
if "${BINARY}" config-show "${INVALID_VALUE_CONFIG_INI}" >/tmp/kage_config_invalid_value.out 2>&1; then
  fail "config com valor invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_config_invalid_value.out)" "valor invalido"

echo "[80/93] rejeita config corrompida"
CORRUPTED_CONFIG_INI="${TEST_DIR}/config_corrupted.ini"
printf '[system]\nversion=0.1.0\nlinha_sem_separador\n' > "${CORRUPTED_CONFIG_INI}"
if "${BINARY}" config-show "${CORRUPTED_CONFIG_INI}" >/tmp/kage_config_corrupted.out 2>&1; then
  fail "config corrompida deveria falhar"
fi
assert_contains "$(cat /tmp/kage_config_corrupted.out)" "arquivo corrompido"

echo "[81/93] rejeita secao invalida em config"
INVALID_SECTION_CONFIG_INI="${TEST_DIR}/config_invalid_section.ini"
printf '[unknown]\nvalue=1\n' > "${INVALID_SECTION_CONFIG_INI}"
if "${BINARY}" config-show "${INVALID_SECTION_CONFIG_INI}" >/tmp/kage_config_invalid_section.out 2>&1; then
  fail "config com secao invalida deveria falhar"
fi
assert_contains "$(cat /tmp/kage_config_invalid_section.out)" "secao invalida"

echo "[82/93] escreve log INFO"
TEST_LOG="${TEST_DIR}/logs/qkage.log"
LOG_STOCK_CSV="${TEST_DIR}/log_stock.csv"
QKAGE_LOG_PATH="${TEST_LOG}" "${BINARY}" add "Log arroz" "Alimentos" 1 "kg" 1 7.5 2027-12-31 1 "${LOG_STOCK_CSV}" >/dev/null
assert_contains "$(cat "${TEST_LOG}")" "[INFO]"
assert_contains "$(cat "${TEST_LOG}")" "[CsvInventoryWriter]"

echo "[83/93] escreve log ERROR"
if QKAGE_LOG_PATH="${TEST_LOG}" "${BINARY}" add "" "Alimentos" 1 "kg" 1 7.5 2027-12-31 1 "${LOG_STOCK_CSV}" >/tmp/kage_log_error.out 2>&1; then
  fail "cadastro invalido deveria falhar e gerar log ERROR"
fi
assert_contains "$(cat "${TEST_LOG}")" "[ERROR]"

echo "[84/93] filtra logs por nivel"
filtered_logs="$(QKAGE_LOG_PATH="${TEST_LOG}" "${BINARY}" logs-show ERROR)"
assert_contains "${filtered_logs}" "[ERROR]"
if [[ "${filtered_logs}" == *"[INFO]"* ]]; then
  fail "logs-show ERROR nao deveria exibir INFO"
fi

echo "[85/93] cria diretorio de logs automaticamente"
AUTO_LOG_PATH="${TEST_DIR}/auto_logs/qkage.log"
AUTO_LOG_STOCK_CSV="${TEST_DIR}/auto_log_stock.csv"
QKAGE_LOG_PATH="${AUTO_LOG_PATH}" "${BINARY}" add "Log feijao" "Alimentos" 1 "kg" 1 8.5 2027-12-31 1 "${AUTO_LOG_STOCK_CSV}" >/dev/null
[[ -f "${AUTO_LOG_PATH}" ]] || fail "logger nao criou arquivo de log automaticamente"

echo "[86/93] rotaciona log acima de 5 MB"
ROTATE_LOG_DIR="${TEST_DIR}/rotate_logs"
ROTATE_LOG_PATH="${ROTATE_LOG_DIR}/qkage.log"
ROTATE_STOCK_CSV="${TEST_DIR}/rotate_stock.csv"
mkdir -p "${ROTATE_LOG_DIR}"
perl -e 'print "x" x (5 * 1024 * 1024 + 1)' > "${ROTATE_LOG_PATH}"
QKAGE_LOG_PATH="${ROTATE_LOG_PATH}" "${BINARY}" add "Log cafe" "Alimentos" 1 "pacote" 1 15 2027-12-31 1 "${ROTATE_STOCK_CSV}" >/dev/null
[[ -f "${ROTATE_LOG_DIR}/qkage_old.log" ]] || fail "logger nao rotacionou qkage.log antigo"
assert_contains "$(cat "${ROTATE_LOG_PATH}")" "Log cafe"

echo "[87/93] rejeita nivel invalido em logs-show"
if QKAGE_LOG_PATH="${TEST_LOG}" "${BINARY}" logs-show BAD >/tmp/kage_logs_invalid_level.out 2>&1; then
  fail "logs-show com nivel invalido deveria falhar"
fi
assert_contains "$(cat /tmp/kage_logs_invalid_level.out)" "nivel de log invalido"

echo "[88/93] lista codigos de erro"
error_codes_output="$("${BINARY}" error-codes)"
assert_contains "${error_codes_output}" "CSV_NOT_FOUND"
assert_contains "${error_codes_output}" "INVALID_CONFIG"
assert_contains "${error_codes_output}" "acao recomendada"

echo "[89/93] dispatcher executa comando valido"
dispatcher_valid_output="$("${BINARY}" error-codes)"
assert_contains "${dispatcher_valid_output}" "Catalogo central de erros"

echo "[90/93] dispatcher rejeita comando inexistente"
if "${BINARY}" comando-inexistente >/tmp/kage_dispatcher_unknown.out 2>&1; then
  fail "comando inexistente deveria falhar"
fi
assert_contains "$(cat /tmp/kage_dispatcher_unknown.out)" "INVALID_COMMAND"

echo "[91/93] dispatcher mostra help sem argumentos"
dispatcher_no_args_output="$("${BINARY}")"
assert_contains "${dispatcher_no_args_output}" "KAGE Home Supply - Ajuda da CLI"
assert_contains "${dispatcher_no_args_output}" "help"

echo "[92/93] dispatcher mostra comando help"
dispatcher_help_output="$("${BINARY}" help)"
assert_contains "${dispatcher_help_output}" "comando"
assert_contains "${dispatcher_help_output}" "rotation-advice"

echo "[93/93] dispatcher rejeita argumentos insuficientes"
if "${BINARY}" add "Arroz" >/tmp/kage_dispatcher_missing_args.out 2>&1; then
  fail "comando com argumentos insuficientes deveria falhar"
fi
assert_contains "$(cat /tmp/kage_dispatcher_missing_args.out)" "argumentos insuficientes"

echo "Todos os testes CSV passaram."
