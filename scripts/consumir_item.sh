#!/usr/bin/env bash
#
# KAGE Home Supply - Consumo de Item
# Autor: Anderson S. Nogueira
# Versao: 1.0.0
#
# Changelog:
# - 1.0.0: Registro de consumo com atualizacao da quantidade em CSV.

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CSV_FILE="${PROJECT_ROOT}/data/estoque.csv"

usage() {
  cat <<'USAGE'
Uso:
  consumir_item.sh "item" quantidade_consumida

Exemplo:
  consumir_item.sh "Papel higienico" 2
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
  [[ $# -eq 2 ]] || { usage; fail "informe exatamente 2 argumentos."; }
  [[ -f "${CSV_FILE}" ]] || fail "arquivo CSV nao encontrado: ${CSV_FILE}"

  local item_alvo="$1"
  local consumo="$2"
  local tmp_file
  tmp_file="$(mktemp)"

  [[ -n "${item_alvo}" ]] || fail "item nao pode ser vazio."
  is_number "${consumo}" || fail "quantidade_consumida deve ser numerica."

  awk -F',' -v OFS=',' -v item_alvo="${item_alvo}" -v consumo="${consumo}" '
    NR == 1 { print; next }
    {
      item=$1
      gsub(/^"|"$/, "", item)
      gsub(/""/, "\"", item)
      if (tolower(item) == tolower(item_alvo)) {
        nova=$3 - consumo
        if (nova < 0) {
          printf "Erro: consumo maior que o estoque atual para %s.\n", item_alvo > "/dev/stderr"
          exit 3
        }
        $3=nova
        found=1
      }
      print
    }
    END {
      if (!found) {
        printf "Erro: item nao encontrado: %s\n", item_alvo > "/dev/stderr"
        exit 2
      }
    }
  ' "${CSV_FILE}" > "${tmp_file}" || {
    local status=$?
    rm -f "${tmp_file}"
    exit "${status}"
  }

  mv "${tmp_file}" "${CSV_FILE}"
  echo "Consumo registrado: ${item_alvo} (-${consumo})"
}

main "$@"
