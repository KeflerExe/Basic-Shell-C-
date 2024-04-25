/// Universidad de La Laguna
/// Escuela Superior de Ingeniería y Tecnología
/// Grado en Ingeniería Informática
/// Asignatura: Sistemas Operativos
/// Curso: 2º
/// Autor: Joel Oscar Martín Gutiérrez
/// Correo: alu0101473840@ull.es
/// Fecha: 11/12/2022
/// Archivo: copyfile_main.cc

#include <iostream>
#include <system_error>
#include <string>

#include "copyfile_func.h"

int main(int argc, char *argv[]) {
  int estado_ultimo_comando{0};
  std::string linea_introducida;
  std::error_code error_read;
  while (true) {
    if (isatty(STDOUT_FILENO)) {
      imprimir_prompt(estado_ultimo_comando);
    }
    error_read = read_line(STDIN_FILENO, linea_introducida);
    if (error_read) {
      exit(EXIT_FAILURE);
    }
    std::vector<shell::command> comandos;
    if (linea_introducida.empty() == false && linea_introducida[0] != '\n') {
      comandos = parse_line(linea_introducida);
    }
    if (!comandos.empty()) {
      auto [valor_retorno, exit_solicitado] = execute_commands(comandos);
      if (exit_solicitado) {
        return valor_retorno;
      }
      estado_ultimo_comando = valor_retorno;
      comandos.clear();
      linea_introducida.clear();
    }
  }
}

