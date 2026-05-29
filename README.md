# iOS Security Memory Safety Demo

Demostracion academica escrita en C sobre conceptos de seguridad relacionados con la cadena de arranque de iOS, la vulnerabilidad checkm8, el modelo de confianza BootROM -> iBoot -> Kernel y los limites entre jailbreak, Secure Enclave y Activation Lock.

Este proyecto es exclusivamente educativo. No reproduce checkm8, no interactua con dispositivos Apple reales, no implementa un bypass de Activation Lock y no contiene codigo operativo de explotacion contra iOS.

## Responsible security research / educational only

Este repositorio debe entenderse como una practica academica de seguridad defensiva y analisis conceptual.

Uso previsto:

- explicar errores de memoria de tipo use-after-free;
- mostrar por que una vulnerabilidad temprana en la cadena de arranque es grave;
- diferenciar compromiso local, Secure Enclave y Activation Lock;
- practicar compilacion, sanitizers y organizacion modular de un proyecto en C.

Uso no previsto:

- atacar dispositivos reales;
- evadir Activation Lock;
- acceder a datos de terceros;
- construir herramientas de jailbreak operativas;
- saltarse mecanismos antirrobo o de autenticacion.

## Contexto

checkm8 es el nombre de una vulnerabilidad asociada a determinados SoCs de Apple, desde A5 hasta A11, relacionada con la BootROM/SecureROM. Su relevancia tecnica se debe a que afecta a una etapa muy temprana de la cadena de arranque.

En un modelo simplificado, la cadena de arranque segura puede representarse asi:

```text
BootROM -> iBoot -> Kernel -> Userland
   \
    \-> Secure Enclave separado
```

La BootROM actua como raiz de confianza. Si se compromete esta primera etapa, se rompe la confianza sobre las etapas posteriores, ya que el atacante puede alterar que componente se carga despues. Ademas, al estar la BootROM grabada en ROM dentro del SoC, un fallo en esa zona no puede corregirse completamente mediante una simple actualizacion de software en dispositivos ya fabricados.

## Lenguaje utilizado

El proyecto esta desarrollado en lenguaje C.

Se ha elegido C porque permite representar de forma directa conceptos de bajo nivel como:

- reserva y liberacion manual de memoria con `malloc` y `free`;
- punteros colgantes;
- estructuras en memoria;
- punteros a funcion;
- secuestro conceptual del flujo de ejecucion;
- errores de tipo use-after-free;
- deteccion de errores mediante AddressSanitizer.

Estos conceptos son adecuados para explicar vulnerabilidades en componentes de bajo nivel, como bootloaders, parsers USB, drivers o codigo de arranque.

## Estructura del proyecto

```text
.
├── include/
│   └── demo.h
├── src/
│   ├── main.c
│   ├── uaf_demo.c
│   ├── bootchain_model.c
│   └── activation_lock_model.c
├── tests/
│   └── smoke_test.c
├── .github/
│   └── workflows/
│       └── build.yml
├── Makefile
├── LICENSE
└── README.md
```

### Modulos principales

| Archivo | Responsabilidad |
| --- | --- |
| `include/demo.h` | Tipos compartidos y prototipos publicos del proyecto |
| `src/main.c` | Punto de entrada y orquestacion de la demo |
| `src/uaf_demo.c` | Demostracion use-after-free, version corregida y ruta ASan |
| `src/bootchain_model.c` | Modelo BootROM -> iBoot -> Kernel y alcance del compromiso |
| `src/activation_lock_model.c` | Modelo conceptual de Activation Lock y catalogo de vectores |
| `tests/smoke_test.c` | Prueba minima para validar el esqueleto de tests |

## Makefile

El proyecto incluye un `Makefile` con los objetivos principales:

```bash
make
make run
make clean
make asan
```

### Compilar

```bash
make
```

Equivale a compilar el proyecto modular usando `gcc`:

```bash
gcc -std=c99 -O0 -Wall -Wextra -Iinclude src/main.c src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c -o demo
```

### Ejecutar

```bash
make run
```

Ejecuta:

```bash
./demo
```

### Limpiar binarios generados

```bash
make clean
```

Elimina los binarios generados por la compilacion normal, AddressSanitizer y pruebas.

### Ejecutar con AddressSanitizer

```bash
make asan
```

El objetivo `asan` compila usando:

```bash
gcc -std=c99 -O0 -Wall -Wextra -fsanitize=address -Iinclude src/main.c src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c -o demo_asan
```

Despues ejecuta una ruta educativa especifica:

```bash
./demo_asan --asan-trigger
```

Esta ruta provoca un acceso use-after-free minimo para que AddressSanitizer lo detecte de forma clara. El objetivo esta preparado para que el workflow pueda continuar aunque ASan reporte el error esperado.

## Sample output

La ejecucion normal muestra una salida similar a esta:

```text
Estado inicial del dispositivo:
  BootROM .............. INTACTO
  iBoot ................ INTACTO
  Kernel iOS ........... INTACTO
  Secure Enclave (SEP) . INTACTO
  Activation Lock ...... INTACTO

=== [1] DEMO: use-after-free con puntero a funcion ===
Peticion legitima creada en direccion: 0x55f...
  [LEGITIMO] Procesando comando: DFU_UPLOAD
Memoria liberada, pero el puntero antiguo sigue existiendo: 0x55f...
Heap grooming simulado: bloque reutilizado en spray[0] (0x55f...)
El programa vuelve a usar el puntero antiguo:
  [SECUESTRADO] Flujo desviado. Mensaje: datos_controlados

=== [2] PROPAGACION POR LA CADENA DE ARRANQUE ===
BootROM comprometida -> se puede cargar iBoot modificado.
iBoot comprometido   -> se puede cargar kernel modificado.
Secure Enclave: dominio independiente -> permanece INTACTO.
Activation Lock: verificacion remota -> permanece INTACTO.
```

La direccion de memoria y el indice `spray[n]` pueden variar entre ejecuciones.

## Que demuestra el codigo

La demo muestra que un fallo de tipo use-after-free puede permitir que un programa vuelva a usar memoria ya liberada. Si esa memoria es reutilizada con datos controlados, el flujo de ejecucion puede desviarse.

En el ejemplo, una estructura `DFURequest` contiene un puntero a funcion llamado `handler`. Primero apunta a una funcion legitima. Despues, la estructura se libera, pero el puntero antiguo se sigue usando. Si el bloque de memoria se reutiliza con una estructura controlada, el campo `handler` puede apuntar a otra funcion.

Esto representa de forma didactica como una vulnerabilidad de memoria puede afectar al flujo de ejecucion. No representa el funcionamiento real de checkm8, sino una abstraccion segura del tipo de problema.

## Relacion conceptual con checkm8

La relacion con checkm8 es conceptual:

- checkm8 afecta a una etapa temprana del arranque;
- el fallo se asocia al entorno USB DFU de la BootROM;
- al estar en ROM, no puede parchearse completamente por software en chips ya fabricados;
- comprometer la BootROM permite alterar la cadena de arranque local;
- este compromiso puede facilitar jailbreaks o ejecucion de codigo no firmado;
- no equivale automaticamente a descifrar datos del usuario ni a retirar Activation Lock.

## Secure Enclave

El codigo modela el Secure Enclave como un dominio separado.

La idea central es que comprometer el procesador principal no implica obtener automaticamente el control del Secure Enclave. En dispositivos reales, el Secure Enclave tiene su propia cadena de arranque, memoria y sistema interno.

Por eso, aunque un atacante consiga ejecutar codigo no autorizado en el procesador principal, no obtiene necesariamente las claves protegidas por el SEP ni puede descifrar automaticamente todos los datos del usuario.

## Activation Lock

Activation Lock se representa como una validacion que depende de un servidor remoto y de la identidad del propietario.

La demo diferencia entre:

- modificar el estado local del dispositivo;
- ocultar una pantalla o parchear una comprobacion local;
- obtener una autorizacion remota legitima.

El punto importante es que jailbreak y Activation Lock no son equivalentes. Un jailbreak puede modificar restricciones locales del sistema, pero Activation Lock depende de la asociacion entre el dispositivo, el Apple ID propietario y la infraestructura remota de activacion.

Por ese motivo, el codigo no implementa ningun bypass real de Activation Lock. Solo muestra, de forma academica, por que un compromiso local no basta para generar una activacion valida.

## Catalogo de vulnerabilidades analizadas

La demo incluye un pequeno catalogo conceptual de vectores relacionados con Activation Lock:

| Vector | Capa atacada | Impacto conceptual |
| --- | --- | --- |
| Parche local de interfaz | Sistema local | Puede ocultar una pantalla, pero no elimina el vinculo remoto |
| Modificacion de servicios locales | Kernel / servicios locales | Puede alterar comportamiento local, pero no genera activacion valida |
| Suplantacion de servidor | Comunicacion cliente-servidor | Debe fallar si se validan certificados y respuestas firmadas |
| Replay de ticket | Protocolo de activacion | Debe fallar si el ticket esta ligado a sesion, ECID y nonces |
| Robo de credenciales | Cuenta del usuario | Puede permitir retirada legitima, pero no es consecuencia directa de checkm8 |
| Fallo logico en servidor | Infraestructura remota | Seria critico, pero no seria un fallo local del dispositivo |
| Ingenieria social | Usuario propietario | Ataque humano, no vulnerabilidad tecnica de BootROM |

## Por que el comportamiento del heap puede variar

El resultado de una demostracion use-after-free basada en reutilizacion de memoria no siempre es identico en todos los entornos.

Algunos factores que influyen:

- el allocator utilizado por la libc, por ejemplo glibc `malloc`, jemalloc, tcmalloc o el heap de Windows;
- caches internas del allocator, como `tcache` en glibc;
- arquitectura del procesador y alineacion de memoria;
- nivel de optimizacion del compilador;
- flags de seguridad y depuracion;
- AddressSanitizer u otros instrumentadores;
- estado previo del heap antes de la asignacion;
- tamano exacto de la estructura y clase de tamaño usada por el allocator.

En muchas implementaciones modernas, un bloque liberado puede reutilizarse rapidamente si se solicita otra reserva del mismo tamaño. Sin embargo, eso no es una garantia portable del lenguaje C. Es un comportamiento dependiente de la implementacion, por eso la demo imprime la direccion del bloque original y la direccion del bloque reutilizado.

La version con AddressSanitizer es mas apropiada para demostrar el error de memoria de forma diagnostica, porque ASan instrumenta las asignaciones y marca regiones liberadas como invalidas. Por eso puede detectar explicitamente el acceso despues de `free`.

## GitHub Actions

El repositorio incluye un workflow en:

```text
.github/workflows/build.yml
```

El workflow ejecuta:

```bash
make
make run
make test
make asan
make clean
```

Esto permite comprobar automaticamente que el proyecto compila y que la ruta educativa de AddressSanitizer sigue funcionando.

## Limitaciones

Este proyecto tiene las siguientes limitaciones intencionadas:

- no interactua con dispositivos Apple reales;
- no usa USB ni modo DFU real;
- no reproduce checkm8;
- no contiene shellcode;
- no implementa cadenas ROP;
- no evade Activation Lock;
- no accede a datos reales del usuario;
- no modifica ningun sistema operativo.

La finalidad es exclusivamente academica y conceptual.

## Licencia

Este proyecto se distribuye bajo licencia MIT. Consulta el archivo `LICENSE` para mas informacion.

## Conclusiones

El proyecto demuestra que una vulnerabilidad en una capa temprana puede tener un impacto muy alto porque rompe la cadena de confianza desde la raiz. Sin embargo, tambien muestra que un sistema moderno separa responsabilidades entre distintos dominios de seguridad.

Por tanto, comprometer BootROM, iBoot o el kernel local no significa necesariamente comprometer Secure Enclave, descifrar datos protegidos o retirar Activation Lock de forma legitima.

El analisis de seguridad debe distinguir entre:

- fallos locales de memoria;
- fallos de cadena de arranque;
- fallos criptograficos;
- fallos de servidor;
- robo de credenciales;
- ingenieria social.

Esa separacion es clave para entender correctamente el alcance real de vulnerabilidades como checkm8.
