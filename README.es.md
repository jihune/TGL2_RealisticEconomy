# Realistic Economy

Realistic Economy es un mod para This Grand Life 2. El mod cambia las reglas del dinero del juego. Impide la ganancia que puedes conseguir cuando cargas una partida. Añade datos a la ventana de Bolsa y funciones automáticas a la ventana de negocio. Corrige algunos fallos del juego.

- Versión del mod: 1.0.0
- Versión del juego: solo v1.03.22
- El desarrollador del juego no ha hecho este mod.

Otros idiomas: [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Qué hace el mod

### Interés de los préstamos
El tipo de interés de un préstamo es el tipo de interés del Banco Central más un margen. La calificación crediticia de tu hogar fija el margen. Hay seis calificaciones: AAA, AA, A, BBB, BB y B.

El mod calcula la calificación con estos cuatro valores:

- la deuda comparada con los activos
- los pagos de préstamos de un año comparados con los ingresos de un año
- el efectivo comparado con los gastos de un año
- el patrimonio neto

En una hipoteca, el margen es mayor cuando pides prestada una parte mayor del precio de la casa. La ventana de préstamos muestra la calificación.

### Efectivo para el fin de mes
El resumen mensual es la ventana que se abre cuando cambia el mes. El mod pone la línea "Fin de mes: efectivo necesario $X" al principio de esta ventana. El importe es la suma de los pagos de tus préstamos y de los otros gastos que pagaste en el último fin de mes. Los salarios y el alquiler son gastos de este tipo.

### Bloqueo de operaciones después de cargar
Si cargas una partida anterior a un fin de mes que ya pasaste, el mod bloquea las operaciones. Durante el bloqueo no puedes comprar ni vender acciones. Tampoco puedes operar con futuros. El bloqueo termina cuando pasas otra vez ese fin de mes. El bloqueo no dura más de dos meses.

Sin el bloqueo, puedes mirar los precios de las acciones del mes siguiente, cargar una partida antigua y comprar acciones. Puedes desactivar esta función en el archivo de configuración.

### Casino
El mod da a los cuatro juegos una apuesta fija y probabilidades fijas.

| Juego | Apuesta | Resultado |
|---|---|---|
| Tragaperras | $1,000 | 22 %: recibes 3 veces la apuesta. 0,8 %: recibes 40 veces la apuesta. |
| Ruleta | $10,000 | 44 %: 2 veces. 2 %: 4 veces. |
| Blackjack | $100,000 | 45 %: 2 veces. 2 %: 2,5 veces. |
| Bacará | $1,000,000 | 46,5 %: 2 veces. |

- Puedes jugar a cada juego una vez al mes.
- Recibes o pierdes el dinero cuando termina la acción del casino.
- Si guardas y cargas otra vez, el resultado es el mismo.
- Si pierdes y después cargas una partida antigua, el mod quita otra vez el dinero perdido.
- Las apuestas suben con los precios del juego.

### Ventana de Bolsa
El mod muestra estos valores de cada empresa:

- PBR
- PER
- rentabilidad por dividendo
- cambio del precio de la acción desde el mes pasado
- precio justo (solo de una empresa que has investigado en el juego)

Selecciona una empresa para ver los valores en la primera pestaña. También puedes poner el puntero del ratón sobre una empresa de la lista.

El mod muestra un aviso en una empresa que está cerca de la quiebra, de una reducción de tamaño o de una emisión de acciones nuevas. Si tienes acciones de esa empresa, el resumen mensual también muestra el aviso.

Suscripción de investigación:

- Haz Mayús+clic en una empresa de la lista. El mod investiga esa empresa otra vez cada mes. Pagas una cuota por cada investigación.
- Para suscribir todas las empresas, haz Ctrl+Mayús+clic en una empresa.

Otros cambios:

- Cuatro de los cinco botones de orden tienen un orden nuevo:
  - mayor capitalización
  - precio más bajo comparado con el precio justo
  - precio más alto comparado con el precio justo
  - mayor subida desde el mes pasado
- El gráfico de una empresa se abre solo con la línea del precio de la acción.
- Los gráficos tienen una vista de 6 meses.
- La lista de futuros muestra la tasa de inflación y la tasa de inflación esperada de cada elemento.

### Salida a bolsa y puestos en el Consejo
En el juego, la salida a bolsa de tu negocio te da el 25 % de las acciones. No recibes efectivo. Con el mod, conservas el 45 % de las acciones. El mod vende el otro 55 % al precio de salida y te da el efectivo. Este efectivo es ingreso sujeto a impuestos de ese año.

Cada 20 % de las acciones de una empresa cotizada garantiza un puesto en el Consejo de Administración. El Consejo tiene cinco puestos. Nomina a un miembro de tu hogar en el mes de la elección. Si no nominas a un miembro, no recibes un puesto.

### Candidatos y ofertas de contrato
La ventana de contratación muestra el pago por hora efectiva de cada candidato. Pago por hora efectiva = pago por hora ÷ eficiencia laboral. Ejemplo: $64.84 ÷ 114 % = $56.88. La lista muestra primero al candidato con el valor más bajo.

Los candidatos de un mes no cambian cuando cargas una partida.

Pon el puntero del ratón sobre el icono de pago de una oferta de contrato. El mod muestra qué porcentaje paga la oferta por encima del coste estándar del trabajo.

Si un negocio no tiene activos suficientes, su eficiencia laboral baja. El resumen mensual muestra entonces el nombre de ese negocio.

### Automatización de un negocio
El mod puede hacer cinco tareas para un negocio. Tú inicias cada tarea en cada negocio.

| Tarea | Cómo se inicia | Qué hace el mod |
|---|---|---|
| Empleados | Activa el modo de gestión automática del juego. Su icono es la flecha circular de la pestaña de empleados. | Contrata a un candidato cuando un puesto no tiene empleados suficientes. Despide al empleado más caro cuando un puesto tiene demasiados empleados durante tres meses. Cuando un empleado pide más salario, lo sustituye por un candidato más barato. Si no hay un candidato así, acepta la petición. |
| Activos | Marca en el negocio la casilla que compra activos nuevos automáticamente. | Compra también los activos que faltan. |
| Anuncios | Haz Mayús+clic en un icono de anuncio. | Activa los anuncios de pago cuando la conciencia es menor del 103 %. Los desactiva cuando la conciencia es del 103 % o más. |
| Contratos | Haz Mayús+clic en el icono de contratos. | Al principio de cada mes, firma las ofertas que pagan más que su coste estándar. Firma tantas ofertas como admite el negocio. Solo lo hace cuando las tareas Empleados y Activos también están activas. |
| Todo | Haz Ctrl+Mayús+clic en un icono de anuncio. | Hace las cuatro tareas anteriores. Alquila más superficie cuando la superficie no es suficiente. Bloquea los clics que cambian el negocio a mano. |

- Para parar una tarea, haz el mismo clic otra vez.
- El mod cobra una cuota cuando contrata a un empleado, mantiene anuncios activos o firma un contrato.
- El mod no hace nada en un negocio cerrado.

### Propiedades por debajo de su valor
Si una propiedad en venta tiene un precio mucho más bajo que su valor, el resumen mensual muestra una línea. La línea da la dirección y la ganancia. La ganancia es el valor menos el precio y los gastos de compra.

### Educación terminada
El juego baja cada mes un 1 % el valor de un título o un certificado. Después de cinco años queda aproximadamente el 55 %. Con el mod, la educación terminada de un miembro de tu hogar conserva su valor. Solo baja la parte que supera el requisito más alto de un empleo. La experiencia del trabajo baja como en el juego.

### Fallos del juego corregidos
- El juego se cerraba cuando cargabas partidas unas decenas de veces sin reiniciar. El mod corrige este fallo.
- Algunos textos traducidos eran incorrectos. El mod los corrige. Ejemplo: el interés fijo aparecía como "Corregido" y ahora aparece como "Fijo".
- Algunas letras de los nombres de personas aparecían como recuadros. El mod muestra las letras correctas.

## Antes de la instalación
- Si una actualización cambia la versión del juego, el mod se desactiva solo. Espera una versión nueva del mod.
- Un antivirus puede avisar de `version.dll`. Este archivo es el Ultimate ASI Loader, que es público. Carga el mod cuando se inicia el juego.
- Cuando cargas por primera vez una partida anterior al mod, el mod desactiva dos interruptores en todos los negocios. Son el modo de gestión automática de los empleados y la compra automática de activos. Con el mod, estos dos interruptores hacen más tareas. Actívalos otra vez solo en los negocios que el mod debe llevar.
- El mod no añade ventanas ni botones. Los textos del mod aparecen en las ventanas del juego, en el idioma del juego.

## Instalación
1. Cierra el juego.
2. Descarga el archivo zip de [Releases](../../releases).
3. Extrae el archivo zip en la carpeta del juego. La carpeta del juego es la carpeta que contiene `TGL2.exe`.
4. Haz doble clic en `RealisticEconomy_install.bat`. Este archivo copia primero tus partidas a la carpeta `saves_before_RealisticEconomy_1`.
5. Inicia el juego.
6. Mira el texto de versión en la esquina inferior derecha del menú principal. Si el texto contiene "+ Realistic Economy", el mod está instalado.

## Desinstalación
1. Cierra el juego.
2. Haz doble clic en `RealisticEconomy_uninstall.bat`.

Puedes abrir sin el mod las partidas que guardaste con el mod.

## Configuración y manual
- El archivo de configuración es `RealisticEconomy.ini`. Después de la instalación está en la carpeta del juego. En este archivo puedes desactivar cada función.
- El manual completo está en inglés: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Da todas las cifras y cuotas de las reglas. El archivo zip también lo contiene.

## Licencia y código fuente
La licencia es MIT. El código fuente está en la carpeta [src](src).
