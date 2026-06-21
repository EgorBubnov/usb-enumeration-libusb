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
