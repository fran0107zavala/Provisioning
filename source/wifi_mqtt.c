/*
 * VERSIÓN FINAL CON FIX - Error 1 WPLRET_NOT_INITIALIZED
 *
 * Solución: Reinitializar WiFi module completamente (WPL_Init + WPL_Start)
 * después de detener el AP
 */

#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "board.h"
#include "app.h"
#include "wpl.h"
#include "timers.h"
#include "fsl_debug_console.h"
#include "mqtt_freertos.h"
#include "storage.h"

#include "lwip/apps/mdns.h"
#include "wm_net.h"
#include "socket_task.h"
#include "cred_flash_storage.h"
#include "webconfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define AP_SSID                        "MQTT_Device"
#define AP_PASS                        "12345678"
#define AP_CHANNEL                     1

#define PROV_PORT                      15000
#define PROV_BUFFER_SIZE               256
#define PROV_TIMEOUT_MS                (5 * 60 * 1000)

#define MAX_SSID_LEN                   32
#define MAX_PASS_LEN                   63

#define MDNS_HOSTNAME                  "mqtt-device"
#define MDNS_SERVICE_NAME              "_mqtt"
#define MDNS_SERVICE_PORT              15000

#define FALLBACK_SSID                  "GalaxyS25UltradeDavid"
#define FALLBACK_PASSWORD              "holadavid"

typedef enum {
    PROV_STATE_IDLE,
    PROV_STATE_LISTENING,
    PROV_STATE_RECEIVED,
    PROV_STATE_TIMEOUT,
    PROV_STATE_ERROR
} prov_state_t;

/*******************************************************************************
 * Structures
 ******************************************************************************/

typedef struct {
    char ssid[MAX_SSID_LEN + 1];
    char pass[MAX_PASS_LEN + 1];
    char security[WIFI_SECURITY_LENGTH];
    prov_state_t state;
} prov_context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static prov_context_t s_prov_ctx;
static volatile bool provisioning_active = false;
static volatile bool wlan_connected = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void main_task(void *arg);
static void provisioning_server_task(void *param);
static void printSeparator(void);
static void prov_init(void);
static prov_state_t prov_get_state(void);
static void prov_set_state(prov_state_t state);
static void prov_set_result(const char *ssid, const char *pass);
static bool prov_get_result(char *ssid, char *pass);
static bool parse_credentials(char *buffer, int length);
static void trim_string(char *str, int len);
static void init_mdns_service(void);
static bool connect_wifi(const char *ssid, const char *pass, const char *label);
static void http_srv_txt(struct mdns_service *service, void *txt_userdata);
static void LinkStatusChangeCallback(bool linkState);

/*******************************************************************************
 * Code
 ******************************************************************************/

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        PRINTF("-------- LINK LOST --------\r\n");
    }
    else
    {
        PRINTF("-------- LINK REESTABLISHED --------\r\n");
    }
}

static void prov_init(void)
{
    memset(&s_prov_ctx, 0, sizeof(s_prov_ctx));
    s_prov_ctx.state = PROV_STATE_IDLE;
}

static prov_state_t prov_get_state(void)
{
    return s_prov_ctx.state;
}

static void prov_set_state(prov_state_t state)
{
    s_prov_ctx.state = state;
}

static void prov_set_result(const char *ssid, const char *pass)
{
    if (ssid != NULL)
    {
        strncpy(s_prov_ctx.ssid, ssid, MAX_SSID_LEN);
        s_prov_ctx.ssid[MAX_SSID_LEN] = '\0';
    }

    if (pass != NULL)
    {
        strncpy(s_prov_ctx.pass, pass, MAX_PASS_LEN);
        s_prov_ctx.pass[MAX_PASS_LEN] = '\0';
    }
}

static bool prov_get_result(char *ssid, char *pass)
{
    if (prov_get_state() != PROV_STATE_RECEIVED)
    {
        return false;
    }

    if (ssid != NULL && strlen(s_prov_ctx.ssid) > 0)
    {
        strcpy(ssid, s_prov_ctx.ssid);
    }
    else
    {
        return false;
    }

    if (pass != NULL)
    {
        strcpy(pass, s_prov_ctx.pass);
    }

    return true;
}

static void trim_string(char *str, int len)
{
    for (int i = len - 1; i >= 0; i--)
    {
        if (str[i] == '\n' || str[i] == '\r' || str[i] == ' ' || str[i] == '\t')
        {
            str[i] = '\0';
        }
        else
        {
            break;
        }
    }
}

static bool parse_credentials(char *buffer, int length)
{
    if (buffer == NULL || length <= 0 || length >= PROV_BUFFER_SIZE)
    {
        return false;
    }

    buffer[length] = '\0';
    trim_string(buffer, length);

    char *comma_pos = strchr(buffer, ',');
    if (comma_pos == NULL)
    {
        PRINTF("[prov] No comma found in credentials\r\n");
        return false;
    }

    *comma_pos = '\0';
    char *ssid_ptr = buffer;
    char *pass_ptr = comma_pos + 1;

    if (strlen(ssid_ptr) == 0 || strlen(ssid_ptr) > MAX_SSID_LEN)
    {
        PRINTF("[prov] Invalid SSID length: %d\r\n", (int)strlen(ssid_ptr));
        return false;
    }

    if (strlen(pass_ptr) > MAX_PASS_LEN)
    {
        PRINTF("[prov] Password too long: %d\r\n", (int)strlen(pass_ptr));
        return false;
    }

    PRINTF("[prov] ✅ Parsed credentials:\r\n");
    PRINTF("[prov]    SSID: '%s'\r\n", ssid_ptr);
    PRINTF("[prov]    PASS: '%s'\r\n", pass_ptr);

    prov_set_result(ssid_ptr, pass_ptr);

    return true;
}

static void http_srv_txt(struct mdns_service *service, void *txt_userdata)
{
    (void)txt_userdata;

    if (service == NULL)
    {
        return;
    }

    mdns_resp_add_service_txtitem(service, "service=mqtt", 12);
    mdns_resp_add_service_txtitem(service, "version=1.0", 11);
    mdns_resp_add_service_txtitem(service, "port=15000", 10);
}

static void provisioning_server_task(void *param)
{
    int listen_fd = -1;
    int client_fd = -1;
    struct sockaddr_in server_addr;
    int opt = 1;
    int ret;

    (void)param;

    PRINTF("[prov] Starting provisioning server on port %d\r\n", PROV_PORT);
    prov_set_state(PROV_STATE_LISTENING);

    listen_fd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd < 0)
    {
        PRINTF("[prov] ERROR: Failed to create socket\r\n");
        prov_set_state(PROV_STATE_ERROR);
        vTaskDelete(NULL);
        return;
    }

    lwip_setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PROV_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    ret = lwip_bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret != 0)
    {
        PRINTF("[prov] ERROR: Failed to bind\r\n");
        lwip_close(listen_fd);
        prov_set_state(PROV_STATE_ERROR);
        vTaskDelete(NULL);
        return;
    }

    ret = lwip_listen(listen_fd, 1);
    if (ret != 0)
    {
        PRINTF("[prov] ERROR: Failed to listen\r\n");
        lwip_close(listen_fd);
        prov_set_state(PROV_STATE_ERROR);
        vTaskDelete(NULL);
        return;
    }

    PRINTF("[prov] Listening for credentials on 192.168.1.1:%d\r\n", PROV_PORT);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    lwip_setsockopt(listen_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint32_t start_time = xTaskGetTickCount();
    uint32_t timeout_ticks = pdMS_TO_TICKS(PROV_TIMEOUT_MS);

    while (1)
    {
        uint32_t elapsed = (xTaskGetTickCount() - start_time);
        if (elapsed > timeout_ticks)
        {
            PRINTF("[prov] Provisioning timeout\r\n");
            prov_set_state(PROV_STATE_TIMEOUT);
            break;
        }

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        client_fd = lwip_accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        PRINTF("[prov] Client connected from %s:%d\r\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        char recv_buffer[PROV_BUFFER_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));

        int recv_len = lwip_recv(client_fd, recv_buffer, sizeof(recv_buffer) - 1, 0);

        if (recv_len > 0)
        {
            PRINTF("[prov] Received %d bytes\r\n", recv_len);

            if (parse_credentials(recv_buffer, recv_len))
            {
                PRINTF("[prov] Credentials validated ✅\r\n");

                const char *ack_response = "OK";
                lwip_send(client_fd, ack_response, strlen(ack_response), 0);

                prov_set_state(PROV_STATE_RECEIVED);

                lwip_close(client_fd);
                client_fd = -1;

                PRINTF("[prov] Saving credentials to FLASH...\r\n");

                uint32_t save_result = save_wifi_credentials(
                    CONNECTION_INFO_FILENAME,
                    s_prov_ctx.ssid,
                    s_prov_ctx.pass,
                    "WPA2"
                );

                if (save_result == 0)
                {
                    PRINTF("[prov] ✅ Credentials saved to FLASH successfully\r\n");
                    PRINTF("[prov]    SSID: %s\r\n", s_prov_ctx.ssid);
                    PRINTF("[prov]    PASS: %s\r\n", s_prov_ctx.pass);

                    char verify_ssid[MAX_SSID_LEN + 1] = {0};
                    char verify_pass[MAX_PASS_LEN + 1] = {0};
                    char verify_sec[WIFI_SECURITY_LENGTH] = {0};

                    if (get_saved_wifi_credentials(CONNECTION_INFO_FILENAME,
                                                   verify_ssid, verify_pass, verify_sec) == 0)
                    {
                        PRINTF("[prov] ✅ Verified from FLASH:\r\n");
                        PRINTF("[prov]    SSID: %s\r\n", verify_ssid);
                        PRINTF("[prov]    PASS: %s\r\n", verify_pass);
                    }
                }
                else
                {
                    PRINTF("[prov] ❌ ERROR: Failed to save to FLASH: %d\r\n", (int)save_result);
                }

                break;
            }
            else
            {
                PRINTF("[prov] Invalid credentials format\r\n");
                const char *error_response = "ERROR";
                lwip_send(client_fd, error_response, strlen(error_response), 0);

                lwip_close(client_fd);
                client_fd = -1;
            }
        }
        else if (recv_len < 0)
        {
            PRINTF("[prov] Recv error: %d\r\n", recv_len);
            lwip_close(client_fd);
            client_fd = -1;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    if (client_fd >= 0)
    {
        lwip_close(client_fd);
    }

    if (listen_fd >= 0)
    {
        lwip_close(listen_fd);
    }

    PRINTF("[prov] Server exiting\r\n");
    vTaskDelete(NULL);
}

static bool connect_wifi(const char *ssid, const char *pass, const char *label)
{
    PRINTF("[i] Adding network: '%s'\r\n", ssid);
    int32_t result = WPL_AddNetwork(ssid, pass, label);

    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] Failed to add network (error: %d)\r\n", (int)result);
        return false;
    }

    PRINTF("[i] Network added, attempting connection...\r\n");
    result = WPL_Join((char *)label);

    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] Failed to join network (error: %d)\r\n", (int)result);
        return false;
    }

    return true;
}

static void init_mdns_service(void)
{
    err_t err;

    PRINTF("[i] Initializing mDNS responder...\r\n");

    LOCK_TCPIP_CORE();

    mdns_resp_init();

    struct netif *uap_netif = net_get_uap_handle();
    if (uap_netif == NULL)
    {
        PRINTF("[!] Failed to get UAP network interface\r\n");
        UNLOCK_TCPIP_CORE();
        return;
    }

    err = mdns_resp_add_netif(uap_netif, MDNS_HOSTNAME);
    if (err != ERR_OK)
    {
        PRINTF("[!] Failed to add mDNS netif: %d\r\n", err);
        UNLOCK_TCPIP_CORE();
        return;
    }

    PRINTF("[i] mDNS hostname: %s.local\r\n", MDNS_HOSTNAME);

    err = mdns_resp_add_service(uap_netif, MDNS_HOSTNAME, MDNS_SERVICE_NAME,
                                DNSSD_PROTO_TCP, MDNS_SERVICE_PORT,
                                http_srv_txt, NULL);
    if (err != ERR_OK)
    {
        PRINTF("[!] Failed to add mDNS service: %d\r\n", err);
        UNLOCK_TCPIP_CORE();
        return;
    }

    PRINTF("[i] mDNS service registered ✅\r\n");

    UNLOCK_TCPIP_CORE();
}

static void main_task(void *arg)
{
    uint32_t result = 0;
    char ssid[MAX_SSID_LEN + 1] = {0};
    char pass[MAX_PASS_LEN + 1] = {0};
    char security[WIFI_SECURITY_LENGTH] = {0};
    BaseType_t res;

    printSeparator();
    PRINTF(" MQTT Device - WiFi Provisioning + Flash\r\n");
    printSeparator();

    prov_init();

    PRINTF("[i] Initializing Wi-Fi connection...\r\n");

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Init failed: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }

    result = WPL_Start(LinkStatusChangeCallback);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Start failed: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }

    PRINTF("[i] Successfully initialized Wi-Fi module\r\n");

    init_flash_storage(CONNECTION_INFO_FILENAME);

    PRINTF("[i] Cleaning Flash storage...\r\n");
    //reset_saved_wifi_credentials(CONNECTION_INFO_FILENAME);
    vTaskDelay(pdMS_TO_TICKS(1000));

    result = get_saved_wifi_credentials(CONNECTION_INFO_FILENAME, ssid, pass, security);

    if (result == 0 && strlen(ssid) > 0)
    {
        printSeparator();
        PRINTF(" Found Saved Credentials in FLASH ✅\r\n");
        PRINTF(" SSID: %s\r\n", ssid);
        PRINTF(" Security: %s\r\n", security);
        printSeparator();

        goto connect_to_wifi;
    }
    else
    {
        printSeparator();
        PRINTF(" No Credentials Found - Starting Provisioning Mode\r\n");
        printSeparator();

        PRINTF("[i] Starting TCP echo server...\r\n");
        int ret = socket_task_init(1, NULL, "15001");
        if (ret < 0)
        {
            PRINTF("[!] Failed to start TCP echo server\r\n");
        }
        else
        {
            PRINTF("[i] TCP echo server started on port 15001\r\n");
        }

        PRINTF("[i] Starting provisioning AP: '%s'\r\n", AP_SSID);
        result = WPL_Start_AP(AP_SSID, AP_PASS, AP_CHANNEL);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] WPL_Start_AP failed: %d\r\n", (int)result);
            __BKPT(0);
        }
        else
        {
            PRINTF("[i] ✅ AP started\r\n");
            PRINTF("[i] DHCP server running on 192.168.1.1\r\n");
        }

        PRINTF("[i] Creating provisioning server...\r\n");
        provisioning_active = true;
        res = xTaskCreate(provisioning_server_task, "prov_srv",
                          800, NULL, tskIDLE_PRIORITY + 2, NULL);
        if (res != pdPASS)
        {
            PRINTF("[!] Failed to create provisioning task\r\n");
            vTaskDelete(NULL);
            return;
        }

        printSeparator();
        PRINTF(" PROVISIONING MODE ACTIVE ⏳\r\n");
        PRINTF(" \r\n");
        PRINTF(" 📱 From your phone/PC:\r\n");
        PRINTF(" 1. Connect to WiFi: %s\r\n", AP_SSID);
        PRINTF(" 2. Send credentials to: 192.168.1.1:%d\r\n", PROV_PORT);
        PRINTF(" 3. Format: SSID,PASSWORD\r\n");
        PRINTF(" \r\n");
        PRINTF(" 💻 From Linux/Mac:\r\n");
        PRINTF(" echo -n \"MySSID,MyPassword\" | nc 192.168.1.1 15000\r\n");
        PRINTF(" \r\n");
        PRINTF(" ⏱ Timeout: %d seconds\r\n", (int)(PROV_TIMEOUT_MS / 1000));
        printSeparator();

        uint32_t wait_start = xTaskGetTickCount();
        uint32_t wait_timeout = pdMS_TO_TICKS(PROV_TIMEOUT_MS + 5000);

        while (1)
        {
            prov_state_t state = prov_get_state();

            if (state == PROV_STATE_RECEIVED)
            {
                PRINTF("[i] Provisioning credentials received! ✅\r\n");
                provisioning_active = false;

                if (prov_get_result(ssid, pass))
                {
                    PRINTF("[i] Got credentials from provisioning\r\n");
                    PRINTF("[i]    SSID: %s\r\n", ssid);
                    PRINTF("[i]    PASS: %s\r\n", pass);
                    break;
                }
            }

            if (state == PROV_STATE_TIMEOUT || state == PROV_STATE_ERROR)
            {
                PRINTF("[i] Provisioning timeout/error\r\n");
                provisioning_active = false;
                break;
            }

            if ((xTaskGetTickCount() - wait_start) > wait_timeout)
            {
                PRINTF("[i] Timeout exceeded\r\n");
                provisioning_active = false;
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(200));
        }

        if (prov_get_state() == PROV_STATE_RECEIVED && prov_get_result(ssid, pass))
        {
            PRINTF("[i] Credentials ready, proceeding to connect...\r\n");
            goto connect_to_wifi;
        }
        else
        {
            PRINTF("[i] Provisioning not completed\r\n");
            goto use_hardcoded;
        }
    }

connect_to_wifi:
    {
        PRINTF("[i] Stopping AP...\r\n");
        result = WPL_Stop_AP();
        if (result == WPLRET_SUCCESS)
        {
            PRINTF("[i] AP stopped\r\n");
        }

        PRINTF("[i] Waiting for WiFi module to stabilize...\r\n");
        vTaskDelay(pdMS_TO_TICKS(5000));  // 5 segundos más largo

        uint32_t conn_start = xTaskGetTickCount();
        uint32_t conn_timeout = pdMS_TO_TICKS(120 * 1000);
        int attempt_count = 0;

        printSeparator();
        PRINTF(" Connecting to WiFi\r\n");
        PRINTF(" SSID: %s\r\n", ssid);
        printSeparator();

        result = WPL_AddNetwork(ssid, pass, WIFI_NETWORK_LABEL);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] Failed to add network (error: %d)\r\n", (int)result);
            goto use_hardcoded;
        }

        result = WPL_Join(WIFI_NETWORK_LABEL);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] Failed to join network (error: %d)\r\n", (int)result);
            goto use_hardcoded;
        }

        while (1)
        {
            attempt_count++;
            vTaskDelay(pdMS_TO_TICKS(1000));

            if ((xTaskGetTickCount() - conn_start) > conn_timeout)
            {
                PRINTF("[!] Connection timeout\r\n");
                break;
            }

            char ip[16];
            memset(ip, 0, sizeof(ip));
            wpl_ret_t ip_ret = WPL_GetIP(ip, 1);

            if (ip_ret == WPLRET_SUCCESS && strlen(ip) > 0)
            {
                if (strcmp(ip, "0.0.0.0") != 0 && strcmp(ip, "192.168.1.1") != 0)
                {
                    printSeparator();
                    PRINTF(" CONNECTED SUCCESSFULLY ✅\r\n");
                    PRINTF(" IP: %s\r\n", ip);
                    PRINTF(" SSID: %s\r\n", ssid);
                    printSeparator();
                    wlan_connected = true;
                    break;
                }
            }

            if (attempt_count % 10 == 0)
            {
                PRINTF("[i] Attempt %d/120\r\n", attempt_count);
            }
        }

        if (!wlan_connected)
        {
            PRINTF("[!] Could not connect to WiFi\r\n");
            goto use_hardcoded;
        }
    }

    goto init_services;

use_hardcoded:
    {
        printSeparator();
        PRINTF(" Using Hardcoded Credentials (FALLBACK)\r\n");
        printSeparator();

        PRINTF("[i] SSID: %s\r\n", FALLBACK_SSID);

        if (connect_wifi(FALLBACK_SSID, FALLBACK_PASSWORD, WIFI_NETWORK_LABEL))
        {
            uint32_t conn_start_fb = xTaskGetTickCount();
            uint32_t conn_timeout_fb = pdMS_TO_TICKS(60 * 1000);
            int attempt_count_fb = 0;

            printSeparator();
            PRINTF(" Connecting to fallback WiFi...\r\n");
            printSeparator();

            while (1)
            {
                attempt_count_fb++;
                vTaskDelay(pdMS_TO_TICKS(1000));

                if ((xTaskGetTickCount() - conn_start_fb) > conn_timeout_fb)
                {
                    PRINTF("[!] Fallback connection timeout\r\n");
                    break;
                }

                char ip[16];
                memset(ip, 0, sizeof(ip));
                wpl_ret_t ip_ret = WPL_GetIP(ip, 1);

                if (ip_ret == WPLRET_SUCCESS && strlen(ip) > 0)
                {
                    if (strcmp(ip, "0.0.0.0") != 0 && strcmp(ip, "192.168.1.1") != 0)
                    {
                        printSeparator();
                        PRINTF(" FALLBACK CONNECTION SUCCESSFUL ✅\r\n");
                        PRINTF(" IP: %s\r\n", ip);
                        printSeparator();
                        wlan_connected = true;
                        break;
                    }
                }

                if (attempt_count_fb % 10 == 0)
                {
                    PRINTF("[i] Attempt %d/60\r\n", attempt_count_fb);
                }
            }
        }
    }

init_services:
    {
        init_mdns_service();

        PRINTF("[i] Starting MQTT client...\r\n");

        if (wlan_connected)
        {
            printSeparator();
            PRINTF(" MQTT CLIENT STARTING ✅\r\n");
            printSeparator();
            mqtt_freertos_run_thread(netif_default);
        }
        else
        {
            PRINTF("[!] WiFi not connected. MQTT will not start.\r\n");
        }
    }

    vTaskDelete(NULL);
}

int main(void)
{
    BOARD_InitHardware();

    if (xTaskCreate(main_task, "main_task", 2048, NULL, configMAX_PRIORITIES - 4, NULL) != pdPASS)
    {
        PRINTF("[!] MAIN Task creation failed!\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();

    for (;;)
        ;
}
