#include <Arduino.h>

#include <espMqttManager.h>

// openssl req -out client.csr -key client.key -new
// https://test.mosquitto.org/ssl/
const char clientCert[] = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDejCCAmKgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\n" \
"FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\n" \
"CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\n" \
"ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMzAxMzEwNzE0\n" \
"NDRaFw0yMzA1MDEwNzE0NDRaMFQxCzAJBgNVBAYTAkJFMRAwDgYDVQQIDAdBTlRX\n" \
"RVJQMRYwFAYDVQQKDA1lc3BNcXR0Q2xpZW50MRswGQYDVQQDDBJlc3BNcXR0Q2xp\n" \
"ZW50LXRlc3QwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCl9fOAG07b\n" \
"kYHyVcf316/qYhj0NBn1m8vHGk4pAinIm6fiFi+d2TmVQ3OqlM816LJxMNdvEMCT\n" \
"0Y0t3BarxnqQUi/rWLiMAVSU5UfV3nhN7+zQ6H+CznXKgaaEMskh4OhrVSqv9dqQ\n" \
"hc2r9aqziDCP6WyJP+cisAo5vLbEiEu9cQ4brH4sk6kWV5NRqNt/QQVNtitg7fLr\n" \
"wxhE6kbfoiO2s17sGjoFD+169wzFggpB5GoSZqkzCkA/txnyqzLt+/SFkViw3D/Q\n" \
"jq5S0ZwlEf9Mpgpsc21EbdGQCyoov6IWNiiQHYpBbrfBGdPsS4YIwLVGjVRW10s/\n" \
"1cOBSA1pSmtjAgMBAAGjGjAYMAkGA1UdEwQCMAAwCwYDVR0PBAQDAgXgMA0GCSqG\n" \
"SIb3DQEBCwUAA4IBAQAigzg4FTKi5nHgrrUBAt6EN1GC+q/8ynOK2mG/me2ghgg5\n" \
"uvvf1oFpBQQGjqtKA93Lvvdrmvao5wtXMXyHl1SttjGfUS/Zw8BizNEgbLpsLHye\n" \
"WrZqkWvKKKxqxKOCoVmEHQakMEe35GdKct+B7e6/xweHDtCg8kxpib18nxGb5zbv\n" \
"zBM98Qzp8VCQAhNh0ytWpeytEjgc6p3N8Z5c3IdGxUWHCMmtmjTzQkwlRQrHCv9g\n" \
"m5SReIkxe95xF5TRZPgSptD6e4hE04E9kZmJNhKmCfuQUj570PF5vtbQaXhToQuR\n" \
"se2TahUZ7zEM+DCmUsvoa7q+JXbd//LPY1cuTGkx\n" \
"-----END CERTIFICATE-----";

// openssl genrsa -out client.key
const char clientKey[] = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEApfXzgBtO25GB8lXH99ev6mIY9DQZ9ZvLxxpOKQIpyJun4hYv\n" \
"ndk5lUNzqpTPNeiycTDXbxDAk9GNLdwWq8Z6kFIv61i4jAFUlOVH1d54Te/s0Oh/\n" \
"gs51yoGmhDLJIeDoa1Uqr/XakIXNq/Wqs4gwj+lsiT/nIrAKOby2xIhLvXEOG6x+\n" \
"LJOpFleTUajbf0EFTbYrYO3y68MYROpG36IjtrNe7Bo6BQ/tevcMxYIKQeRqEmap\n" \
"MwpAP7cZ8qsy7fv0hZFYsNw/0I6uUtGcJRH/TKYKbHNtRG3RkAsqKL+iFjYokB2K\n" \
"QW63wRnT7EuGCMC1Ro1UVtdLP9XDgUgNaUprYwIDAQABAoIBABRRwLZ2oXkQn7Bk\n" \
"AE/AALHZwORlXKhNou2FsvwiX6tII43y2XGiHwK/d3HzNkuLag2hxT679lgi1/49\n" \
"bCkr6+Wvql5ORoodv5VbXSg6TNB+B6isTrBdRt68cP7OmQGk39AEZW2V1GTNfRkI\n" \
"wxWoHwn/smI7iLkehKH9EQOPDHbtJiHi1Uk8O7nYrgytwl859bJlzyqvoKt/MVKy\n" \
"OhaGdAeIgD569tt/IMROWA8U6p0rGuSicRdy77+hOZnH+EXK/bKqCJJ4dqTLA42g\n" \
"fFobNfbHP3PURsjYXxz8yx32Gi9cha2a6m4mWVRvjnk7v2uDSQnpORvaOgGyxRqm\n" \
"OvkOmEECgYEA1l4RnjClD2q8gPMZbMK4OK6Tmqp59Vb9+rNTfzbrsDa+wqxoDerk\n" \
"Pvgr/exMXoD/i35HY0QYYm1fxLafePnG0MT4jt5oKyfgimnbfb40aoPqlJZIetL0\n" \
"qJrtiCAWErSG+2YLRCHT7/8ZtxfPjGUfC9/8mQVabAZCzSLIejGuBwUCgYEAxjEy\n" \
"AAOKY+CNzYpWzQObRjq3USTAtYFgZkAOVQjrNAK/BKwRdain2EKApUN7zmYad3jl\n" \
"qatvNk5anJqwCCCDo/Rq/4hnBzgRq6yY5SWKHgllP1+evXJEEpTEjYb25DNF2+6V\n" \
"NT52jbsUsZs9rVkESsxU6Rk61afwKT47gNzz5UcCgYAD/mqrnNJmcCGk8WKG4TXX\n" \
"cIE1f3mYNY+zzQWAOEC53rs9G0MxBzBLcAo5x95AYHziV27zDYC6zLjdDF36PgUT\n" \
"PKg42THto+eODJzXQeTuGxkSvZ4KE8YVIhXM1HQ00wI0ytjo4wx0yzbqb1ohjny+\n" \
"luS44R2nJ5K1CbHcfPJ2FQKBgHUDNJcHgBxpxq70nzzZhUJYhkTVIyXGZcjYxt1z\n" \
"r5H8kRTMFl7WHYqz8Moh5qW8aKHi8g1TlwyEBEFznrydnbseIlIOEW4Ur3IhLAAW\n" \
"Qs5rE6jv5oPJQ+GIjedVKM0gbH1mBhfycgJJ0vTtR+Phq4QR0YvyBmeAiG2BPZZe\n" \
"XW0fAoGANO+t0T9cJqaQJ8eTt2Nvr2KOGSZztbW91iPRdzHHay+rRorw4GKwGrzL\n" \
"s8LQF/HnT3WIEdb4K3mALU6W5lIZ7PuJU/KNRDNRqB60dOHZxU6NWPGTt0vc5/BK\n" \
"PXNnY3vnlRC3ioM/HF5hGhsFOidGVKXjweO1QEHEIT/gJ3jpOSA=\n" \
"-----END RSA PRIVATE KEY-----";

// https://test.mosquitto.org/ssl/mosquitto.org.crt
const char serverCert[] = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\n" \
"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\n" \
"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\n" \
"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\n" \
"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\n" \
"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\n" \
"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\n" \
"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\n" \
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\n" \
"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\n" \
"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\n" \
"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\n" \
"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\n" \
"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\n" \
"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\n" \
"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n" \
"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\n" \
"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\n" \
"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\n" \
"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\n" \
"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\n" \
"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\n" \
"-----END CERTIFICATE-----";

void setup() {
  delay(10000);
  Serial.begin(115200);
  espMqttManager::setup();

  // add or modify the MQTT clients properties between setup and start
  espMqttManager::mqttClient.setCertificate(clientCert);
  espMqttManager::mqttClient.setPrivateKey(clientKey);
  espMqttManager::mqttClient.setCACert(serverCert);

  espMqttManager::start();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    int8_t signalQuality = WiFi.RSSI();
    char buf[5];
    snprintf(buf, sizeof(buf), "%d", signalQuality);
    espMqttManager::mqttClient.publish("test/topic", 0, false, buf);
  }
  espMqttManager::loop();
}
