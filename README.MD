BUILD
--------

1) Open crossover solution (Crossover.sln) with visual studio and compile.
2) Copy "dependencies" folder content to the binary directory (either debug or release)
3) Create logs directory on binary directory (either debug or release)

RUNNING
--------

1) Edit config.xml for the client(s)
2) Run x64/Release/server
3) Run x64/Release/client

CONFIGURATION
------------

Sample configuration file for the client (client.xml):

```xml
<client key="f7f9122e-04a5-45f1-83b9-4415f9a5a4f8" refreshInterval="1000" alertInterval="5000" host="localhost:8080" mail="crossovertestcpp@gmail.com">
 <alert type="memory" limit="50%" />
 <alert type="cpu" limit="20%" />
 <alert type="processes" limit="50" />
</client>
```

Interval is expressed in milliseconds.

Sample configuration file for the server (server.xml):

```xml
<server port="8080" smtp_host="smtp.gmail.com" smtp_port="587" smtp_login="crossovertestcpp@gmail.com" smtp_password="yD2JaL3gKu2uGhzW" smtp_sender="crossover test" smtp_sender_mail="crossovertestcpp@gmail.com"/>
```

LOGS
------------

Logs will be saved in logs/log.txt under client binary folder

REQUIREMENTS
--------

64 bit machine

LIBRARIES AND RESPECTIVE LICENSES
----------

CSmtp - The Code Project Open License (CPOL)
OpenSSL - OpenSSL License
TinyXML - Zlib License
ASIO network and low-level IO programming -  Boost Software License.
JSON for Modern C++ - MIT License
SQLite Modern Cpp - MIT License
SQlite - Public Domain
SPDLog - MIT License

LICENSE
----------

MIT license - http://www.opensource.org/licenses/mit-license.php