# 🔌 USB Device Enumeration with libusb

> Лабораторная работа: получение списка USB-устройств в Linux с помощью библиотеки **libusb-1.0**, включая чтение серийных номеров из дескриптора устройства. Результаты сверены с утилитой **hardinfo2**.

---

## 📋 Задание

- Набрать и отладить программу на C для получения списка USB-устройств
- Разобраться в коде, добавить вывод серийного номера (`iSerialNumber` дескриптора устройства)
- Установить утилиту `hardinfo` на VirtualBox
- Сверить полученные данные с показаниями `hardinfo`

---

## 🛠️ Стек

| Технология | Описание |
|---|---|
| C | Язык программирования |
| libusb-1.0 | Библиотека для работы с USB-устройствами |
| gcc | Компилятор |
| Ubuntu 22.04 | ОС (в VirtualBox) |
| hardinfo2 | Утилита для просмотра системной информации |

---

## 📁 Структура репозитория

```
.
└── lsusb.c      # Исходный код программы
```

---

## 💻 Исходный код

```c
#include <stdio.h>
#include <sys/types.h>
#include <libusb-1.0/libusb.h>

void print_devs(libusb_device **devs)
{
    libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor\n");
            return;
        }

        /* Вывод базовой информации */
        printf("idVendor=%04x;  idProduct=%04x (bus: %d, addr: %d)",
            desc.idVendor, desc.idProduct,
            libusb_get_bus_number(dev),
            libusb_get_device_address(dev));

        /* Получение серийного номера через iSerialNumber */
        if (desc.iSerialNumber > 0) {
            libusb_device_handle *handle;
            int rc = libusb_open(dev, &handle);
            if (rc == 0) {
                unsigned char serial[256];
                int len = libusb_get_string_descriptor_ascii(
                              handle,
                              desc.iSerialNumber,
                              serial,
                              sizeof(serial));
                if (len > 0) {
                    printf("  Serial: %s", serial);
                } else {
                    printf("  Serial: (read error %d)", len);
                }
                libusb_close(handle);
            } else {
                printf("  Serial: (cannot open device: %s)", libusb_error_name(rc));
            }
        } else {
            printf("  Serial: (none)");
        }
        printf("\n");
    }
}

int main(void)
{
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        return (int) cnt;

    print_devs(devs);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
```

---

## ⚙️ Сборка и запуск

**1. Установите зависимости:**
```bash
sudo apt update
sudo apt install -y gcc libusb-1.0-0-dev
```

**2. Скомпилируйте:**
```bash
gcc -o lsusb lsusb.c -lusb-1.0
```

**3. Запустите** (нужен `sudo` для чтения серийных номеров):
```bash
sudo ./lsusb
```

---

## 🔍 Как работает код

### Ключевые функции libusb

| Функция | Назначение |
|---|---|
| `libusb_init()` | Инициализация контекста libusb |
| `libusb_get_device_list()` | Получение списка всех USB-устройств |
| `libusb_get_device_descriptor()` | Чтение дескриптора устройства |
| `libusb_open()` | Открытие дескриптора для чтения строк |
| `libusb_get_string_descriptor_ascii()` | Чтение строкового дескриптора (серийный номер) |
| `libusb_free_device_list()` | Освобождение списка устройств |
| `libusb_exit()` | Завершение работы с libusb |

### Про поле `iSerialNumber`

Поле `iSerialNumber` в структуре `libusb_device_descriptor` — это **индекс** строкового дескриптора, а не сам серийный номер. Значение `0` означает, что серийный номер отсутствует. Для получения реальной строки необходимо:
1. Открыть устройство через `libusb_open()`
2. Вызвать `libusb_get_string_descriptor_ascii()` с этим индексом

---

## 📊 Результаты

### Вывод программы (`sudo ./lsusb`)

```
idVendor=1d6b;  idProduct=0003 (bus: 2, addr: 1)  Serial: 0000:00:0c.0
idVendor=80ee;  idProduct=0021 (bus: 1, addr: 2)  Serial: (none)
idVendor=1d6b;  idProduct=0002 (bus: 1, addr: 1)  Serial: 0000:00:0c.0
```

### Вывод hardinfo2 (Devices → USB Devices)

| # | Устройство |
|---|---|
| 001:001 | Linux 2.0 root hub |
| 001:002 | Oracle (InnoTek) USB Tablet |
| 002:001 | Linux 3.0 root hub |

### Сверка данных

| idVendor | idProduct | Устройство | Serial (программа) | hardinfo2 |
|---|---|---|---|---|
| `1d6b` | `0002` | Linux 2.0 root hub | `0000:00:0c.0` | Vendor: Linux Foundation |
| `80ee` | `0021` | Oracle USB Tablet | (none) | Oracle (InnoTek) |
| `1d6b` | `0003` | Linux 3.0 root hub | `0000:00:0c.0` | Vendor: Linux Foundation |

**Вывод:** Данные совпадают. Устройства, отображаемые `hardinfo2`, соответствуют устройствам, найденным программой. Серийный номер вида `0000:00:0c.0` у root hub'ов является адресом PCI-контроллера, к которому они подключены — это стандартное поведение виртуальной машины VirtualBox.

---

## 📝 Выводы

В ходе выполнения лабораторной работы:

1. Изучена библиотека **libusb-1.0** и её API для перечисления USB-устройств
2. Реализован вывод **серийного номера** через чтение строкового дескриптора по индексу `iSerialNumber`
3. Установлена утилита **hardinfo2** и получен список USB-устройств через графический интерфейс
4. Данные из обоих источников **совпали** — все три устройства VirtualBox (2× Linux root hub + USB Tablet) найдены корректно
