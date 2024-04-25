/// Universidad de La Laguna
/// Escuela Superior de Ingeniería y Tecnología
/// Grado en Ingeniería Informática
/// Asignatura: Sistemas Operativos
/// Curso: 2º
/// Autor: Joel Oscar Martín Gutiérrez
/// Correo: alu0101473840@ull.es
/// Fecha: 11/12/2022
/// Archivo: copyfile_func.h

#include <iostream>
#include <system_error>
#include <csignal>
#include <vector>
#include <algorithm>
#include <assert.h> 
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <sstream>
#include <sys/wait.h>

namespace shell {
  using command = std::vector<std::string>;
}

namespace shell {
  struct command_result {
    int valor_retorno;
    bool exit_solicitado;
    command_result(int valor_retorno, bool exit_solicitado=false)
      : valor_retorno{valor_retorno}, exit_solicitado{exit_solicitado}
    {}
    static command_result quit(int valor_retorno=0) {
    return command_result{valor_retorno, true};
    }
  };
}

int execute(const std::vector<std::string>& args);
int execute_program(const std::vector<std::string>& args, bool has_wait);
shell::command_result execute_commands(const std::vector<shell::command>& commands);
std::vector<shell::command> parse_line(const std::string& linea_introducida);
std::error_code read_line(int descriptor_archivo, std::string& linea_introducida);
std::error_code print(const std::string& string);
void imprimir_prompt(int estado_ultimo_comando);
int echo_command(const std::vector<std::string>& args);
int cd_command(const std::vector<std::string>& args);
std::error_code read(int descriptor_origen, std::vector<uint8_t>& buffer);
std::error_code write(int descriptor_destino, const std::vector<uint8_t>& buffer);
std::error_code copy_file(const std::string& src_path, const std::string& dst_path, bool preserve_all);
std::error_code move_file(const std::string& src_path, const std::string& dst_path);