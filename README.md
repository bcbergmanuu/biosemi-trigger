The triggerbox has two virtual com ports.

- first to listen to biosemi (ascii char 0 1 2 3 for pin 16,17 (0 based).
  | **Serial** | **Pin 16** | **Pin 17** |
  |:----------:|:----------:|:----------:|
  | 0          | 0          | 0          |
  | 1          | 0          | 1          |
  | 2          | 1          | 0          |
  | 3          | 1          | 1          |

- second to send to biosemi. (Byte 0-255 for pin 0-7).

Device is attached to biosemi usb triggerbox with dsub37 cable. Pinlayout: https://www.biosemi.com/faq/trigger_signals.htm
