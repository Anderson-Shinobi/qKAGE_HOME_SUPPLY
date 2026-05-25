#!/usr/bin/env bash
#
# KAGE Home Supply - Economia Convertida em Investimento
# Autor: Anderson S. Nogueira
# Versao: 1.0.0
#
# Changelog:
# - 1.0.0: Calculo de economia mensal estimada e projecao anual.

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CSV_FILE="${PROJECT_ROOT}/data/estoque.csv"

usage() {
  cat <<'USAGE'
Uso:
  calcular_economia.sh percentual_economia_mensal

O percentual representa a economia estimada por compras planejadas/em fardo.
Exemplo:
  calcular_economia.sh 12.5
USAGE
}

fail() {
  echo "Erro: $*" >&2
  exit 1
}

is_number() {
  [[ "${1}" =~ ^[0-9]+([.][0-9]+)?$ ]]
}

main() {
  [[ $# -eq 1 ]] || { usage; fail "informe o percentual de economia."; }
  [[ -f "${CSV_FILE}" ]] || fail "arquivo CSV nao encontrado: ${CSV_FILE}"

  local percentual="$1"
  is_number "${percentual}" || fail "percentual deve ser numerico."

  awk -F',' -v percentual="${percentual}" '
    NR > 1 {
      consumo=$5 + 0
      preco=$6 + 0
      gasto += consumo * preco
    }
    END {
      economia = gasto * percentual / 100
      printf "Gasto mensal estimado: R$ %.2f\n", gasto
      printf "Economia mensal estimada (%.2f%%): R$ %.2f\n", percentual, economia
      printf "Investimento anual potencial: R$ %.2f\n", economia * 12
    }
  ' "${CSV_FILE}"
}

main "$@"
