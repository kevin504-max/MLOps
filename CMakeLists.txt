# CMakeLists.txt na raiz do seu projeto

set(EXTRA_COMPONENT_DIRS components/esp-idf-lib)

# Inclui os diretórios componentes, onde o ESP-IDF procurará pelos componentes.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(seu_projeto)
cmake_minimum_required(VERSION 3.28)
# Defina a versão mínima do CMake necessária para o projeto