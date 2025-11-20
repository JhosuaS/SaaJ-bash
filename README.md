**SaaJ-bash** es un proyecto escolar de shell minimalista implementado con syscalls, no se hizo uso de funciones de librería, a excepción de `execvp()`, que fue permitida para el uso de comandos externos no declarados dentro del programa.
#### Compilar
>Es necesario que el entorno cuente con una versión de gcc instalada

Ingresar: `gcc SaaJ-bash -o SaaJ-bash` en consola
Ingresar: `make`
##### Targets Adicionales
`make debug`. `make clean`, `make run`
#### Ejecutar
Ingresar: `./SaaJ-bash`
Ingresar: `make run`
El proyecto implementa los siguientes comandos internos:

| Comando | Descripción                                                                                                                                              | Uso                                             | Ejemplo                        |
| ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ------------------------------ |
| ls      | Lista los archivos y ficheros presentes en la ruta de trabajo actual                                                                                     | `ls`                                            |                                |
| cd      | Accede a la ruta especificada                                                                                                                            | `cd <fichero/ruta>`                             | `cd /usr/bin`                  |
| pwd     | Imprime la ruta de trabajo actual                                                                                                                        | `pwd`                                           |                                |
| mkdir   | Crea un nuevo fichero en la ruta de trabajo actual                                                                                                       | `mkdir <nombre_fichero>`                        | `mkdir prueba`                 |
| rm      | Elimina un fichero o archivo                                                                                                                             | `rm <nombre_fichero/nombre_archivo.ext>`        | `rm prueba`<br>`rm prueba.txt` |
| cp      | Copia un archivo                                                                                                                                         | `cp <archivo_origen.ext> <archivo_destino.ext>` | `cp prueba.txt copia.txt`      |
| mv      | Mueve y renombra archivos. (Limitación: el comando no soporta mover un archivo a un directorio existente debido al uso directo de la syscall `rename()`) | `mv <archivo_origen.ext> <archivo_destino>`     | `mv prueba.txt prueba2.txt`    |
| cat     | Muestra el contenido de un archivo                                                                                                                       | `cat <nombre_archivo.ext>`                      | `cat prueba.txt`               |
El shell implementa una función para ejecutar comandos externos, que se encuentren especificados en la variable de entorno `PATH`. Algunos ejemplos son:
- `vim`: editor de texto
- `nano`: editor de texto
- `date`: muestra la fecha y hora del sistema
- `echo`: muestra un mensaje específico en la salida indicada, por defecto: consola