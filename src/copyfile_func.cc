/// Universidad de La Laguna
/// Escuela Superior de Ingeniería y Tecnología
/// Grado en Ingeniería Informática
/// Asignatura: Sistemas Operativos
/// Curso: 2º
/// Autor: Joel Oscar Martín Gutiérrez
/// Correo: alu0101473840@ull.es
/// Fecha: 11/12/2022
/// Archivo: copyfile_func.h

#include "scope.hpp"
#include "copyfile_func.h"

std::error_code read(int descriptor_origen, std::vector<uint8_t>& buffer);

int execute(const std::vector<std::string>& args) {
  std::vector<const char*> args_char;
  int ultimo_comando;
  args_char.clear();
  for (int iterador = 0; iterador < args.size(); ++iterador) {
    args_char.push_back(args[iterador].c_str());
  }
  args_char.push_back(nullptr);
  if (execvp(args_char[0], const_cast<char* const*>(args_char.data())) != 0) {
    return -1;
  }    
  return 0;
}

int execute_program(const std::vector<std::string>& args, bool has_wait=true) {
  pid_t child = fork(); 
  if (child == 0) {
    execute(args);
    return EXIT_FAILURE; 
  } else if (child > 0) {
    int status;
    wait(&status);
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

shell::command_result execute_commands(const std::vector<shell::command>& commands) {
  std::string comando;
  int valor_retorno;
  for (int iterador = 0; iterador < commands.size(); ++iterador) {
    std::vector<std::string> comando_escogido = commands[iterador];
    if (comando_escogido[0] == "exit") {
      int valor_retorno = 0;
      return shell::command_result::quit(valor_retorno);
    } else {
      if (comando_escogido[0] == "echo") {
        valor_retorno = echo_command(comando_escogido);
      } else {
        if (comando_escogido[0] == "cd") {
          valor_retorno = cd_command(comando_escogido);
        } else {
          if (comando_escogido[0] == "cp") {
            if (comando_escogido[1] == "-a") {
              valor_retorno = copy_file(comando_escogido[2], comando_escogido[3], true).value();
            } else {
              valor_retorno = copy_file(comando_escogido[1], comando_escogido[2], false).value();
            }
          } else {
            if (comando_escogido[0] == "mv") {
              valor_retorno = move_file(comando_escogido[1], comando_escogido[2]).value();
            } else {
              valor_retorno = execute_program(comando_escogido);
            }
          }
        }
      }
    } 
  }
  return shell::command_result(valor_retorno, false);
}

std::vector<shell::command> parse_line(const std::string& linea_introducida) {
  std::vector<shell::command> vector_comandos;
  shell::command comando;
  char caracter_final;
  std::istringstream flujo_string(linea_introducida);
  while(!flujo_string.eof()) {
    std::string palabra;
    flujo_string >> palabra;
    if (palabra == ";" || palabra == "&" || palabra == "|") {
      comando.push_back(palabra);
      vector_comandos.push_back(comando);
      comando.clear();
    } else {
      if (palabra.front() == '#') {
        return vector_comandos;
      }
      if (palabra.back() == ';' || palabra.back() == '&' || palabra.back() == '|') {
        caracter_final = palabra.back();
        palabra.pop_back();
        comando.push_back(palabra);
        std::string caracter_final_string(1, caracter_final);
        vector_comandos.push_back(comando);
        comando.clear();
      } else {
        if (!palabra.empty()) {
          comando.push_back(palabra);
        }
      }
    }
  }
  vector_comandos.push_back(comando);
  return vector_comandos;
}

std::error_code read_line(int descriptor_archivo, std::string& linea_introducida) {
  static std::vector<uint8_t> input_pendiente;
  while (true) {
    if (std::find(input_pendiente.begin(), input_pendiente.end(), '\n') != input_pendiente.end()) {
      for (int iterador = 0; iterador < input_pendiente.size(); ++iterador) {
        char caracter_extraido = input_pendiente[iterador];
        if (caracter_extraido == '\n') {
          break;
        }
        linea_introducida = linea_introducida + caracter_extraido;
      }
      input_pendiente.clear();
      return std::error_code(0, std::system_category());
    }
    std::vector<uint8_t> buffer(4ul * 1024);
    read(descriptor_archivo, buffer);
    if (errno) {
      return std::error_code(errno, std::system_category());
    }
    if (buffer.empty()) {
      if (input_pendiente.empty() == false) {
        for (int iterador = 0; iterador < input_pendiente.size(); ++iterador) {
          char caracter_extraido = input_pendiente[iterador];
          linea_introducida = linea_introducida + caracter_extraido;
          if (caracter_extraido == '\000') {
            break;
          }
        }
        linea_introducida = linea_introducida + '\n';
        input_pendiente.clear();
      }
      return std::error_code(0, std::system_category());
    } else {
      input_pendiente.insert(std::end(input_pendiente), std::begin(buffer), std::end(buffer));
    }
  }
  return std::error_code(0, std::system_category());
}


std::error_code print(const std::string& string) {
  int bytes_escritos = write(STDOUT_FILENO, string.c_str(), string.size());
  if (bytes_escritos == -1) {
    return std::error_code(errno, std::system_category());
  }
  return std::error_code(0, std::system_category());
}

int echo_command(const std::vector<std::string>& args) {
  std::string cadena_resultante;
  std::error_code error_print;
  for (int i = 1; i < args.size(); ++i) {
    cadena_resultante = cadena_resultante + args[i] + " ";
  }
  cadena_resultante = cadena_resultante + '\n';
  error_print = print(cadena_resultante);
  if (error_print) {
    std::cerr << error_print.message() << std::endl;
    return -1;
  } else {
    return 0;
  }
}

int cd_command(const std::vector<std::string>& args) {
  std::error_code error_cd;
  if (chdir(args[1].c_str())) {
    std::cerr << error_cd.message() << std::endl;
    return -1;
  } 
  return 0;
}

void imprimir_prompt(int estado_ultimo_comando) {
  std::string mensaje_prompt;
  char hostname_pointer[256];
  gethostname(hostname_pointer, sizeof(hostname_pointer)); 
  std::string hostname = hostname_pointer;
  char ruta_actual_pointer[256];
  getcwd(ruta_actual_pointer, sizeof(ruta_actual_pointer));
  std::string ruta_actual = ruta_actual_pointer;
  mensaje_prompt = mensaje_prompt + getlogin() + "@" + hostname + ":" + ruta_actual + " ";
  if (estado_ultimo_comando == 0) {
    mensaje_prompt = mensaje_prompt + "$> ";
  } else {
    mensaje_prompt = mensaje_prompt + "$< ";
  }
  print(mensaje_prompt);
}

/// @brief Función de lectura que envuelve a la llamada de la función read y comprueba si el número de bytes leidos es correcto
/// @param descriptor_origen 
/// @param buffer 
/// @return Error code 0 si no falla, errno en caso de que la lectura falle
std::error_code read(int descriptor_origen, std::vector<uint8_t>& buffer) {
  ssize_t bytes_leidos = read(descriptor_origen, buffer.data(), buffer.size());
  if (bytes_leidos < 0) {
    return std::error_code(errno, std::system_category());
  }
  buffer.resize(bytes_leidos);
  return std::error_code(0, std::system_category());
}

/// @brief Función de escritura que envuelve a la llamada de la función write y comprueba si el número de bytes escritos es correcto
/// @param descriptor_destino 
/// @param buffer 
/// @return Error code 0 si no falla, errno en caso de que la lectura falle
std::error_code write(int descriptor_destino, const std::vector<uint8_t>& buffer) {
  ssize_t bytes_escritos = write(descriptor_destino, buffer.data(), buffer.size());
  if (bytes_escritos < buffer.size()) {
    write(descriptor_destino, buffer.data(), buffer.size());
  }
  return std::error_code(0,std::system_category());
}

/// @brief Función copy_file encargada de copiar un archivo de una ruta origen en una ruta destino y, en caso de solicitarlo, copiar también sus permisos
/// @param src_path 
/// @param dst_path 
/// @param preserve_all 
/// @return Error code 0 si no falla, errno en caso de que la lectura falle
std::error_code copy_file(const std::string& src_path, const std::string& dst_path, bool preserve_all) {
  struct stat informacion_origen;
  struct stat informacion_destino;
  std::string dst_path_modificada;
  if (stat(src_path.c_str(), &informacion_origen) == 0 && S_ISREG(informacion_origen.st_mode)) {
  } else {
    return std::error_code(errno, std::system_category());
  }
  if (stat(dst_path.c_str(), &informacion_destino) == 0) {
    assert(((informacion_origen.st_dev != informacion_destino.st_dev) && (informacion_origen.st_ino != informacion_destino.st_ino)) || ((informacion_origen.st_dev == informacion_destino.st_dev) && (informacion_origen.st_ino != informacion_destino.st_ino)));
    if (S_ISDIR(informacion_destino.st_mode)) {
      char* copia_src_path = const_cast<char*>(src_path.c_str());
      std::string nombre_archivo_ruta = basename(copia_src_path);
      dst_path_modificada = dst_path + nombre_archivo_ruta;
    }
  }
  int descriptor_origen = open(src_path.c_str(), O_RDONLY);
  if (descriptor_origen < 0) {
    return std::error_code(errno, std::system_category());
  }
  auto guard_origen = scope::scope_exit([descriptor_origen] { close(descriptor_origen); });
  int descriptor_destino{0};
  if (stat(dst_path_modificada.c_str(), &informacion_destino) == 0) {
    descriptor_destino = open(dst_path_modificada.c_str(), O_WRONLY | O_TRUNC);
  } else {
    descriptor_destino = open(dst_path_modificada.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
  }
  if (descriptor_destino < 0) {
    return std::error_code(errno, std::system_category());
  }
  auto guard_destino = scope::scope_exit([descriptor_destino] { close(descriptor_destino); });
  std::vector<uint8_t> buffer(16ul * 1024 * 1024);
  while (buffer.size() != 0) {
    std::error_code error_lectura = read(descriptor_origen, buffer);
    if (error_lectura) {
      return error_lectura;
    }
    std::error_code error_escritura = write(descriptor_destino, buffer);
    if (error_escritura) {
      return error_escritura; 
    }
  }
  if (preserve_all == true) {
    struct utimbuf acceso_y_modificacion;
    acceso_y_modificacion.actime = informacion_origen.st_atime;
    acceso_y_modificacion.modtime = informacion_origen.st_mtime;
    utime(dst_path_modificada.c_str(), &acceso_y_modificacion);
    chown(dst_path_modificada.c_str(), informacion_origen.st_uid, informacion_origen.st_gid);
    chmod(dst_path_modificada.c_str(), informacion_origen.st_mode);
  }
  return std::error_code(0, std::system_category());
}

std::error_code move_file(const std::string& src_path, const std::string& dst_path) {
  struct stat informacion_origen;
  struct stat informacion_destino;
  std::string dst_path_modificada;
  if (stat(src_path.c_str(), &informacion_origen) == 0) {
  } else {
    return std::error_code(errno, std::system_category());
  }
  if (stat(dst_path.c_str(), &informacion_destino) == 0) {
    if (S_ISDIR(informacion_destino.st_mode)) {
      char* copia_src_path = const_cast<char*>(src_path.c_str());
      std::string nombre_archivo_ruta = basename(copia_src_path);
      dst_path_modificada = dst_path + nombre_archivo_ruta;
    }
  }
  if (rename(src_path.c_str(), dst_path_modificada.c_str()) == 0) {
  } else {
    copy_file(src_path, dst_path_modificada, true);
    unlink(src_path.c_str());
  }
  return std::error_code(0, std::system_category());
}