# RokuPal 1.0.0 — correcciones del fork

Esta revisión conserva la arquitectura propia del fork y no depende de asumir comportamiento de Drupal 6 estándar.

## Cambios funcionales

Se consolidó la instalación en `install.php`, con SQLite como valor predeterminado y selección explícita de SQLite, MySQL/MariaDB o PostgreSQL mediante la URL de conexión. El motor carga el driver correspondiente y conserva el perfil RokuPal.

La activación tardía de `rokupal_forum` ahora intenta crear la estructura del foro durante el hook de activación y limpia la caché. Esto evita que el módulo quede habilitado pero incompleto hasta la primera visita a `/forum`.

Se eliminó la pestaña administrativa de desinstalación del menú de módulos. La pantalla principal mantiene el flujo único de activar o desactivar módulos y sus dependencias.

El editor visual de bloques ahora presenta regiones reales del tema como zonas de destino y tarjetas arrastrables. Los cambios de región se envían mediante campos ocultos y se persisten al guardar.

El tema RokuPal declara y renderiza nuevas regiones: preheader, herramientas de cabecera, hero, zonas superior e inferior del contenido, tres zonas tripartitas y zonas superior e inferior del pie.

## Verificación

Se ejecutó `php -l` sobre los 48 archivos PHP del paquete y no se detectaron errores de sintaxis. La validación de ejecución contra Apache/MariaDB/SQLite/PostgreSQL reales no puede sustituir una instalación en el XAMPP del usuario; se recomienda probar primero sobre una copia de la base de datos y revisar permisos de `sites/default` y `sites/default/files`.

La compatibilidad objetivo es PHP 8.0.x con Apache 2.4.x y MariaDB 10.4.x, conforme al entorno XAMPP indicado.
