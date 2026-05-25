#!/usr/bin/env bash
#
# KAGE Home Supply - Cadastro de Item
# Autor: Anderson S. Nogueira
# Versao: 1.0.0
#
# Changelog:
# - 1.0.0: Criacao do cadastro inicial em CSV com validacao de entrada.

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CSV_FILE="${PROJECT_ROOT}/data/estoque.csv"

usage() {
  cat <<'USAGE'
Uso:
  add_item.sh "item" "categoria" quantidade "unidade" consumo_mensal preco_unitario validade estoque_minimo

Exemplo:
  add_item.sh "Papel higienico" "Higiene" 24 "rolo" 8 1.35 2027-12-31 6
USAGE
}

fail() {
  echo "Erro: $*" >&2
  exit 1
}

is_number() {
  [[ "${1}" =~ ^[0-9]+([.][0-9]+)?$ ]]
}

is_date() {
  [[ "${1}" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]
}

csv_escape() {
  local value="${1//\"/\"\"}"
  printf '"%s"' "${value}"
}

ensure_csv() {
  mkdir -p "$(dirname "${CSV_FILE}")"
  if [[ ! -f "${CSV_FILE}" ]]; then
    echo "item,categoria,quantidade,unidade,consumo_mensal,preco_unitario,validade,estoque_minimo" > "${CSV_FILE}"
  fi
}

main() {
  [[ $# -eq 8 ]] || { usage; fail "informe exatamente 8 argumentos."; }

  local item="$1"
  local categoria="$2"
  local quantidade="$3"
  local unidade="$4"
  local consumo_mensal="$5"
  local preco_unitario="$6"
  local validade="$7"
  local estoque_minimo="$8"

  [[ -n "${item}" ]] || fail "item nao pode ser vazio."
  [[ -n "${categoria}" ]] || fail "categoria nao pode ser vazia."
  [[ -n "${unidade}" ]] || fail "unidade nao pode ser vazia."
  is_number "${quantidade}" || fail "quantidade deve ser numerica."
  is_number "${consumo_mensal}" || fail "consumo_mensal deve ser numerico."
  is_number "${preco_unitario}" || fail "preco_unitario deve ser numerico."
  is_date "${validade}" || fail "validade deve estar no formato YYYY-MM-DD."
  is_number "${estoque_minimo}" || fail "estoque_minimo deve ser numerico."

  ensure_csv

  {
    csv_escape "${item}"; printf ','
    csv_escape "${categoria}"; printf ','
    printf '%s,' "${quantidade}"
    csv_escape "${unidade}"; printf ','
    printf '%s,%s,' "${consumo_mensal}" "${preco_unitario}"
    csv_escape "${validade}"; printf ','
    printf '%s\n' "${estoque_minimo}"
  } >> "${CSV_FILE}"

  echo "Item cadastrado com sucesso: ${item}"
  echo "Arquivo atualizado: ${CSV_FILE}"
}

main "$@"
