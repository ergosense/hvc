# Mongoose OS library for integrating with HVC-P

Command specification: https://www.components.omron.com/documents/35730/96820/B5T-007001_CommandSpecifications_A.pdf/420d62f7-d842-238e-3438-5de5bb3e1d16

# Porting

To port this library to other frameworks, you can get rid of the `mgos_hvc` files and implement the required methods:

- `hvc_read_bytes`
- `hvc_write_bytes`
- `hvc_read_bytes_available`
- `hvc_log_info`
- `hvc_log_debug`
- `hvc_log_error`

# Mongoose OS specific functionality

The `mgos_hvc` file implements the Mongoose OS init methods and registers a task that will execute body and face detection at the defined interval. The following events are raised:

## MGOS_HVC_EVENT_DETECTION

Raised when the device detects atleast one person.

## MGOS_HVC_EVENT_INIT

Raised when the device has initialized and all configuration values have been set.