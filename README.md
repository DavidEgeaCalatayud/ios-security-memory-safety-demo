# iCloud Phone Block - Demo academica sobre checkm8

Este repositorio contiene una demostracion academica escrita en C sobre conceptos de seguridad relacionados con la cadena de arranque de iOS, la vulnerabilidad checkm8 y los limites entre jailbreak, Secure Enclave y Activation Lock.

El objetivo del proyecto no es reproducir checkm8 ni construir una herramienta funcional contra dispositivos Apple. El codigo es una simulacion didactica pensada para explicar, de forma controlada, como un fallo de memoria puede comprometer una capa temprana del sistema y por que ese compromiso no implica automaticamente romper todos los mecanismos de seguridad del dispositivo.

## Contexto

checkm8 es el nombre de una vulnerabilidad asociada a determinados SoCs de Apple, desde A5 hasta A11, relacionada con la BootROM/SecureROM. Su importancia se debe a que afecta a una etapa muy temprana de la cadena de arranque.

En terminos conceptuales, la cadena de arranque segura de iOS puede representarse asi:

```text
BootROM / SecureROM
        -> iBoot
        -> Kernel de iOS
        -> Sistema operativo
```

La BootROM actua como raiz de confianza. Si se compromete esta primera etapa, se rompe la confianza sobre las etapas posteriores, ya que el atacante puede alterar que se carga despues. Ademas, al estar la BootROM grabada en ROM dentro del SoC, un fallo en esa zona no puede corregirse completamente mediante una simple actualizacion de software en dispositivos ya fabricados.

## Lenguaje utilizado

El proyecto esta desarrollado en lenguaje C.

Se ha elegido C porque permite representar de forma clara conceptos de bajo nivel como:

- reserva y liberacion manual de memoria con `malloc` y `free`;
- punteros colgantes;
- estructuras en memoria;
- punteros a funcion;
- secuestro conceptual del flujo de ejecucion;
- errores de tipo use-after-free.

Estos conceptos son adecuados para explicar vulnerabilidades en componentes de bajo nivel, como bootloaders, parsers USB, drivers o codigo de arranque.

## Archivo principal

El archivo principal del proyecto es:

```text
demo_checkm8_academico.c
```

Este archivo incluye varias demostraciones:

1. Un ejemplo de use-after-free con puntero a funcion.
2. Una simulacion de heap grooming.
3. Un modelo conceptual de propagacion BootROM -> iBoot -> Kernel.
4. Una explicacion del limite entre compromiso local y Secure Enclave.
5. Una simulacion de Activation Lock como validacion remota.
6. Un catalogo de vectores relacionados con Activation Lock.
7. Una version corregida del use-after-free.

## Que demuestra el codigo

La demo muestra que un fallo de tipo use-after-free puede permitir que un programa vuelva a usar memoria ya liberada. Si esa memoria es reutilizada con datos controlados, el flujo de ejecucion puede desviarse.

En el ejemplo, una estructura `DFURequest` contiene un puntero a funcion llamado `handler`. Primero apunta a una funcion legitima. Despues, la estructura se libera, pero el puntero antiguo se sigue usando. Si el bloque de memoria se reutiliza con una estructura controlada, el campo `handler` puede apuntar a otra funcion.

Esto representa de forma didactica como una vulnerabilidad de memoria puede afectar al flujo de ejecucion. No representa el funcionamiento real de checkm8, sino una abstraccion segura del tipo de problema.

## Relacion con checkm8

La relacion con checkm8 es conceptual:

- checkm8 afecta a una etapa temprana del arranque;
- el fallo se asocia al entorno USB DFU de la BootROM;
- al estar en ROM, no puede parchearse completamente por software en chips ya fabricados;
- comprometer la BootROM permite alterar la cadena de arranque local;
- este compromiso puede facilitar jailbreaks o ejecucion de codigo no firmado;
- no equivale automaticamente a descifrar datos del usuario ni a retirar Activation Lock.

## Secure Enclave

El codigo tambien modela el Secure Enclave como un dominio separado.

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

## Compilacion

Para compilar el programa:

```bash
gcc -std=c99 -O0 -Wall -Wextra demo_checkm8_academico.c -o demo_checkm8_academico
```

La opcion `-O0` evita optimizaciones agresivas del compilador y ayuda a que la demo sea mas predecible.

## Ejecucion

En Linux o macOS:

```bash
./demo_checkm8_academico
```

En Windows con MinGW:

```bash
demo_checkm8_academico.exe
```

## Advertencia sobre los warnings

Es normal que el compilador pueda mostrar advertencias relacionadas con use-after-free. En este proyecto esas advertencias son esperadas, porque una parte de la demo esta disenada precisamente para representar ese error.

La version corregida incluida al final del programa muestra una mitigacion basica: invalidar el puntero despues de liberar la memoria y comprobarlo antes de volver a usarlo.

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
