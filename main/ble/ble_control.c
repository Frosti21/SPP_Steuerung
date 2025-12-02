#include "nvs_flash.h"


#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include <stdio.h>
#include <time.h>
#include <ctype.h> 

#include "ble_control.h"
#include "comms.h"
char *TAG = "BLE-Server";
uint8_t ble_addr_type;

// Lese die Empfangenen Daten und Schreibe auf....
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // printf("Data from the client__: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);

    // char * data = (char *)ctxt->om->om_data;
    // printf("Numern Erzeugung: %d\n",strcmp(data, "Forward\0"));
    int send = 0;
    int send_poti = 0;
    char data[64];
    size_t len = ctxt->om->om_len;

    // Sicherheit: Länge begrenzen, damit data nicht überläuft
    if (len >= sizeof(data)) len = sizeof(data) - 1;

    // 1Daten kopieren
    memcpy(data, ctxt->om->om_data, len);

    //  Nullterminator hinzufügen
    data[len] = '\0';

    // Alles in Kleinbuchstaben umwandeln
    for (size_t i = 0; i < len; i++) {
        data[i] = tolower((unsigned char)data[i]);
    }
    printf("Data from client: %s\n", data);

    if ((strncmp(data, "forward", ctxt->om->om_len) == 0) | (strncmp(data, "f", ctxt->om->om_len) == 0)|
        (strncmp(data, "^", ctxt->om->om_len) == 0)| (strncmp(data, "w", ctxt->om->om_len) == 0))
    {
        ESP_LOGI("GAP", "BLE GAP EVENT - forward");
        send = 1;
        xQueueSend(acc_queue, &send, portMAX_DELAY);
    }
    else if ((strncmp(data, "backward", ctxt->om->om_len) == 0)  | (strncmp(data, "b", ctxt->om->om_len) == 0)|
    (strncmp(data, "v", ctxt->om->om_len) == 0) | (strncmp(data, "s", ctxt->om->om_len) == 0))
    {
        ESP_LOGI("GAP", "BLE GAP EVENT - backward");
        send = 2;
        xQueueSend(acc_queue, &send, portMAX_DELAY);
    }
    else if ((strncmp(data, "break", ctxt->om->om_len) == 0) | (strncmp(data, "br", ctxt->om->om_len) == 0)|
    (strncmp(data, "x", ctxt->om->om_len) == 0) | (strncmp(data, " ", ctxt->om->om_len) == 0))
    {
        ESP_LOGI("GAP", "BLE GAP EVENT - break");
        send = 3;
        xQueueSend(acc_queue, &send, portMAX_DELAY);
    }
    else if ((strncmp(data, "left", ctxt->om->om_len) == 0) | (strncmp(data, "l", ctxt->om->om_len) == 0)|
    (strncmp(data, "<", ctxt->om->om_len) == 0)| (strncmp(data, "a", ctxt->om->om_len) == 0))
    {
        ESP_LOGI("GAP", "BLE GAP EVENT - left");
        send = 4;
        xQueueSend(steering_queue, &send, portMAX_DELAY);
    }
    else if ((strncmp(data, "right", ctxt->om->om_len) == 0) | (strncmp(data, "r", ctxt->om->om_len) == 0)|
    (strncmp(data, ">", ctxt->om->om_len) == 0) | (strncmp(data, "d", ctxt->om->om_len) == 0))
    {
        printf("right\n");
        ESP_LOGI("GAP", "BLE GAP EVENT - right");
        send = 5;
        xQueueSend(steering_queue, &send, portMAX_DELAY);
    }
    else if ((strncmp(data, "off", ctxt->om->om_len) == 0) | (strncmp(data, "o", ctxt->om->om_len) == 0))
    {
        printf("off\n");
        ESP_LOGI("GAP", "BLE GAP EVENT - right");
        send = 6;
        xQueueSend(acc_queue, &send, portMAX_DELAY);
    }
    else if (data[0] == 's'){
        printf("SpeedWerte: %s\n", data);
        send_poti = 0;
        sscanf(data, "s%d", &send_poti); 
        xQueueReset(speed_queue);
        xQueueSend(speed_queue, &send_poti, portMAX_DELAY);
        send_poti = 0;
    }
    else{
        printf("Data from the client??: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
        send = 0;
        xQueueSend(my_queue, &send, portMAX_DELAY);
    }
    return 0;
}

// Read data from ESP32 defined as server
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
    return 0;
}

static int device_status(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "ESP-Ready-For-Move", strlen("ESP-Ready-For-Move"));
    return 0;
}

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
        // hier werden die services hinzugefügt
        {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
            .flags = BLE_GATT_CHR_F_READ,
            .access_cb = device_read},
        {.uuid = BLE_UUID16_DECLARE(0xFFFF),           // Define UUID for reading
                .flags = BLE_GATT_CHR_F_READ,
                .access_cb = device_status},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
        if (event->connect.status !=0)
        {
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    // Booten der BLE
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}

void connect_ble(void)
{
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}