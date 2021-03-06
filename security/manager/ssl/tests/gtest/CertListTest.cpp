/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gtest/gtest.h"
#include "nsCOMPtr.h"
#include "nsIPrefService.h"
#include "nsIX509Cert.h"
#include "nsIX509CertDB.h"
#include "nsNSSCertificate.h"
#include "nsServiceManagerUtils.h"
#include "nsString.h"

// certspec (for pycert.py)
//
// issuer:ca
// subject:ca
// extension:basicConstraints:cA,
// extension:keyUsage:cRLSign,keyCertSign
// serialNumber:1
const char* kCaPem =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICsjCCAZygAwIBAgIBATALBgkqhkiG9w0BAQswDTELMAkGA1UEAwwCY2EwIhgP\n"
    "MjAxNTExMjgwMDAwMDBaGA8yMDE4MDIwNTAwMDAwMFowDTELMAkGA1UEAwwCY2Ew\n"
    "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6iFGoRI4W1kH9braIBjYQ\n"
    "PTwT2erkNUq07PVoV2wke8HHJajg2B+9sZwGm24ahvJr4q9adWtqZHEIeqVap0WH\n"
    "9xzVJJwCfs1D/B5p0DggKZOrIMNJ5Nu5TMJrbA7tFYIP8X6taRqx0wI6iypB7qdw\n"
    "4A8Njf1mCyuwJJKkfbmIYXmQsVeQPdI7xeC4SB+oN9OIQ+8nFthVt2Zaqn4CkC86\n"
    "exCABiTMHGyXrZZhW7filhLAdTGjDJHdtMr3/K0dJdMJ77kXDqdo4bN7LyJvaeO0\n"
    "ipVhHe4m1iWdq5EITjbLHCQELL8Wiy/l8Y+ZFzG4s/5JI/pyUcQx1QOs2hgKNe2N\n"
    "AgMBAAGjHTAbMAwGA1UdEwQFMAMBAf8wCwYDVR0PBAQDAgEGMAsGCSqGSIb3DQEB\n"
    "CwOCAQEAchHf1yV+blE6fvS53L3DGmvxEpn9+t+xwOvWczBmLFEzUPdncakdaWlQ\n"
    "v7q81BPyjBqkYbQi15Ws81hY3dnXn8LT1QktCL9guvc3z4fMdQbRjpjcIReCYt3E\n"
    "PB22Jl2FCm6ii4XL0qDFD26WK3zMe2Uks6t55f8VeDTBGNoPp2JMsWY1Pi4vR6wK\n"
    "AY96WoXS/qrYkmMEOgFu907pApeAeE8VJzXjqMLF6/W1VN7ISnGzWQ8zKQnlp3YA\n"
    "mvWZQcD6INK8mvpZxIeu6NtHaKEXGw7tlGekmkVhapPtQZYnWcsXybRrZf5g3hOh\n"
    "JFPl8kW42VoxXL11PP5NX2ylTsJ//g==\n"
    "-----END CERTIFICATE-----";

// certspec (for pycert.py)
//
// issuer:ca
// subject:ca-intermediate
// extension:basicConstraints:cA,
// extension:keyUsage:cRLSign,keyCertSign
// serialNumber:2
const char* kCaIntermediatePem =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICvzCCAamgAwIBAgIBAjALBgkqhkiG9w0BAQswDTELMAkGA1UEAwwCY2EwIhgP\n"
    "MjAxNTExMjgwMDAwMDBaGA8yMDE4MDIwNTAwMDAwMFowGjEYMBYGA1UEAwwPY2Et\n"
    "aW50ZXJtZWRpYXRlMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuohR\n"
    "qESOFtZB/W62iAY2ED08E9nq5DVKtOz1aFdsJHvBxyWo4NgfvbGcBptuGobya+Kv\n"
    "WnVramRxCHqlWqdFh/cc1SScAn7NQ/weadA4ICmTqyDDSeTbuUzCa2wO7RWCD/F+\n"
    "rWkasdMCOosqQe6ncOAPDY39ZgsrsCSSpH25iGF5kLFXkD3SO8XguEgfqDfTiEPv\n"
    "JxbYVbdmWqp+ApAvOnsQgAYkzBxsl62WYVu34pYSwHUxowyR3bTK9/ytHSXTCe+5\n"
    "Fw6naOGzey8ib2njtIqVYR3uJtYlnauRCE42yxwkBCy/Fosv5fGPmRcxuLP+SSP6\n"
    "clHEMdUDrNoYCjXtjQIDAQABox0wGzAMBgNVHRMEBTADAQH/MAsGA1UdDwQEAwIB\n"
    "BjALBgkqhkiG9w0BAQsDggEBAC0ys8UOmYgvH5rrTeV6u79ocHqdQFwdmR7/4d08\n"
    "i3asC7b70dw0ehA5vi4cq5mwBvQOGZq4wvsR4jSJW0+0hjWL1dr2M6VxmCfjdqhU\n"
    "NQHPlY6y7lLfYQbFfUHX99ZgygJjdmmm7H8MBP4UgPb8jl6Xq53FgYykiX/qPmfb\n"
    "pzpOFHDi+Tk67DLCvPz03UUDYNx1H0OhRimj0DWhdYGUg2DHfLQkOEYvrYG4wYB8\n"
    "AB/0hrG51yFsuXrzhYcinTKby11Qk6PjnOQ/aZvK00Jffep/RHs8lIOWty9SarMG\n"
    "oNSECn+6I9AgStJdo6LuP1QPyrQe3DZtAHhRJAPAoU7BSqM=\n"
    "-----END CERTIFICATE-----";

const uint8_t kCaIntermediateDer[] = {
    0x30, 0x82, 0x02, 0xBF, 0x30, 0x82, 0x01, 0xA9, 0xA0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x01, 0x02, 0x30, 0x0B, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86,
    0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x30, 0x0D, 0x31, 0x0B, 0x30, 0x09, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0C, 0x02, 0x63, 0x61, 0x30, 0x22, 0x18, 0x0F,
    0x32, 0x30, 0x31, 0x35, 0x31, 0x31, 0x32, 0x38, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x38, 0x30, 0x32, 0x30,
    0x35, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5A, 0x30, 0x1A, 0x31, 0x18,
    0x30, 0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x0F, 0x63, 0x61, 0x2D,
    0x69, 0x6E, 0x74, 0x65, 0x72, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x74, 0x65,
    0x30, 0x82, 0x01, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86,
    0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F, 0x00,
    0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xBA, 0x88, 0x51,
    0xA8, 0x44, 0x8E, 0x16, 0xD6, 0x41, 0xFD, 0x6E, 0xB6, 0x88, 0x06, 0x36,
    0x10, 0x3D, 0x3C, 0x13, 0xD9, 0xEA, 0xE4, 0x35, 0x4A, 0xB4, 0xEC, 0xF5,
    0x68, 0x57, 0x6C, 0x24, 0x7B, 0xC1, 0xC7, 0x25, 0xA8, 0xE0, 0xD8, 0x1F,
    0xBD, 0xB1, 0x9C, 0x06, 0x9B, 0x6E, 0x1A, 0x86, 0xF2, 0x6B, 0xE2, 0xAF,
    0x5A, 0x75, 0x6B, 0x6A, 0x64, 0x71, 0x08, 0x7A, 0xA5, 0x5A, 0xA7, 0x45,
    0x87, 0xF7, 0x1C, 0xD5, 0x24, 0x9C, 0x02, 0x7E, 0xCD, 0x43, 0xFC, 0x1E,
    0x69, 0xD0, 0x38, 0x20, 0x29, 0x93, 0xAB, 0x20, 0xC3, 0x49, 0xE4, 0xDB,
    0xB9, 0x4C, 0xC2, 0x6B, 0x6C, 0x0E, 0xED, 0x15, 0x82, 0x0F, 0xF1, 0x7E,
    0xAD, 0x69, 0x1A, 0xB1, 0xD3, 0x02, 0x3A, 0x8B, 0x2A, 0x41, 0xEE, 0xA7,
    0x70, 0xE0, 0x0F, 0x0D, 0x8D, 0xFD, 0x66, 0x0B, 0x2B, 0xB0, 0x24, 0x92,
    0xA4, 0x7D, 0xB9, 0x88, 0x61, 0x79, 0x90, 0xB1, 0x57, 0x90, 0x3D, 0xD2,
    0x3B, 0xC5, 0xE0, 0xB8, 0x48, 0x1F, 0xA8, 0x37, 0xD3, 0x88, 0x43, 0xEF,
    0x27, 0x16, 0xD8, 0x55, 0xB7, 0x66, 0x5A, 0xAA, 0x7E, 0x02, 0x90, 0x2F,
    0x3A, 0x7B, 0x10, 0x80, 0x06, 0x24, 0xCC, 0x1C, 0x6C, 0x97, 0xAD, 0x96,
    0x61, 0x5B, 0xB7, 0xE2, 0x96, 0x12, 0xC0, 0x75, 0x31, 0xA3, 0x0C, 0x91,
    0xDD, 0xB4, 0xCA, 0xF7, 0xFC, 0xAD, 0x1D, 0x25, 0xD3, 0x09, 0xEF, 0xB9,
    0x17, 0x0E, 0xA7, 0x68, 0xE1, 0xB3, 0x7B, 0x2F, 0x22, 0x6F, 0x69, 0xE3,
    0xB4, 0x8A, 0x95, 0x61, 0x1D, 0xEE, 0x26, 0xD6, 0x25, 0x9D, 0xAB, 0x91,
    0x08, 0x4E, 0x36, 0xCB, 0x1C, 0x24, 0x04, 0x2C, 0xBF, 0x16, 0x8B, 0x2F,
    0xE5, 0xF1, 0x8F, 0x99, 0x17, 0x31, 0xB8, 0xB3, 0xFE, 0x49, 0x23, 0xFA,
    0x72, 0x51, 0xC4, 0x31, 0xD5, 0x03, 0xAC, 0xDA, 0x18, 0x0A, 0x35, 0xED,
    0x8D, 0x02, 0x03, 0x01, 0x00, 0x01, 0xA3, 0x1D, 0x30, 0x1B, 0x30, 0x0C,
    0x06, 0x03, 0x55, 0x1D, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF,
    0x30, 0x0B, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x04, 0x04, 0x03, 0x02, 0x01,
    0x06, 0x30, 0x0B, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01,
    0x01, 0x0B, 0x03, 0x82, 0x01, 0x01, 0x00, 0x2D, 0x32, 0xB3, 0xC5, 0x0E,
    0x99, 0x88, 0x2F, 0x1F, 0x9A, 0xEB, 0x4D, 0xE5, 0x7A, 0xBB, 0xBF, 0x68,
    0x70, 0x7A, 0x9D, 0x40, 0x5C, 0x1D, 0x99, 0x1E, 0xFF, 0xE1, 0xDD, 0x3C,
    0x8B, 0x76, 0xAC, 0x0B, 0xB6, 0xFB, 0xD1, 0xDC, 0x34, 0x7A, 0x10, 0x39,
    0xBE, 0x2E, 0x1C, 0xAB, 0x99, 0xB0, 0x06, 0xF4, 0x0E, 0x19, 0x9A, 0xB8,
    0xC2, 0xFB, 0x11, 0xE2, 0x34, 0x89, 0x5B, 0x4F, 0xB4, 0x86, 0x35, 0x8B,
    0xD5, 0xDA, 0xF6, 0x33, 0xA5, 0x71, 0x98, 0x27, 0xE3, 0x76, 0xA8, 0x54,
    0x35, 0x01, 0xCF, 0x95, 0x8E, 0xB2, 0xEE, 0x52, 0xDF, 0x61, 0x06, 0xC5,
    0x7D, 0x41, 0xD7, 0xF7, 0xD6, 0x60, 0xCA, 0x02, 0x63, 0x76, 0x69, 0xA6,
    0xEC, 0x7F, 0x0C, 0x04, 0xFE, 0x14, 0x80, 0xF6, 0xFC, 0x8E, 0x5E, 0x97,
    0xAB, 0x9D, 0xC5, 0x81, 0x8C, 0xA4, 0x89, 0x7F, 0xEA, 0x3E, 0x67, 0xDB,
    0xA7, 0x3A, 0x4E, 0x14, 0x70, 0xE2, 0xF9, 0x39, 0x3A, 0xEC, 0x32, 0xC2,
    0xBC, 0xFC, 0xF4, 0xDD, 0x45, 0x03, 0x60, 0xDC, 0x75, 0x1F, 0x43, 0xA1,
    0x46, 0x29, 0xA3, 0xD0, 0x35, 0xA1, 0x75, 0x81, 0x94, 0x83, 0x60, 0xC7,
    0x7C, 0xB4, 0x24, 0x38, 0x46, 0x2F, 0xAD, 0x81, 0xB8, 0xC1, 0x80, 0x7C,
    0x00, 0x1F, 0xF4, 0x86, 0xB1, 0xB9, 0xD7, 0x21, 0x6C, 0xB9, 0x7A, 0xF3,
    0x85, 0x87, 0x22, 0x9D, 0x32, 0x9B, 0xCB, 0x5D, 0x50, 0x93, 0xA3, 0xE3,
    0x9C, 0xE4, 0x3F, 0x69, 0x9B, 0xCA, 0xD3, 0x42, 0x5F, 0x7D, 0xEA, 0x7F,
    0x44, 0x7B, 0x3C, 0x94, 0x83, 0x96, 0xB7, 0x2F, 0x52, 0x6A, 0xB3, 0x06,
    0xA0, 0xD4, 0x84, 0x0A, 0x7F, 0xBA, 0x23, 0xD0, 0x20, 0x4A, 0xD2, 0x5D,
    0xA3, 0xA2, 0xEE, 0x3F, 0x54, 0x0F, 0xCA, 0xB4, 0x1E, 0xDC, 0x36, 0x6D,
    0x00, 0x78, 0x51, 0x24, 0x03, 0xC0, 0xA1, 0x4E, 0xC1, 0x4A, 0xA3};

// certspec (for pycert.py)
//
// issuer:ca-intermediate
// subject:ca-second-intermediate
// extension:basicConstraints:cA,
// extension:keyUsage:cRLSign,keyCertSign
// serialNumber:3
const char* kCaSecondIntermediatePem =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIC0zCCAb2gAwIBAgIBAzALBgkqhkiG9w0BAQswGjEYMBYGA1UEAwwPY2EtaW50\n"
    "ZXJtZWRpYXRlMCIYDzIwMTUxMTI4MDAwMDAwWhgPMjAxODAyMDUwMDAwMDBaMCEx\n"
    "HzAdBgNVBAMMFmNhLXNlY29uZC1pbnRlcm1lZGlhdGUwggEiMA0GCSqGSIb3DQEB\n"
    "AQUAA4IBDwAwggEKAoIBAQC6iFGoRI4W1kH9braIBjYQPTwT2erkNUq07PVoV2wk\n"
    "e8HHJajg2B+9sZwGm24ahvJr4q9adWtqZHEIeqVap0WH9xzVJJwCfs1D/B5p0Dgg\n"
    "KZOrIMNJ5Nu5TMJrbA7tFYIP8X6taRqx0wI6iypB7qdw4A8Njf1mCyuwJJKkfbmI\n"
    "YXmQsVeQPdI7xeC4SB+oN9OIQ+8nFthVt2Zaqn4CkC86exCABiTMHGyXrZZhW7fi\n"
    "lhLAdTGjDJHdtMr3/K0dJdMJ77kXDqdo4bN7LyJvaeO0ipVhHe4m1iWdq5EITjbL\n"
    "HCQELL8Wiy/l8Y+ZFzG4s/5JI/pyUcQx1QOs2hgKNe2NAgMBAAGjHTAbMAwGA1Ud\n"
    "EwQFMAMBAf8wCwYDVR0PBAQDAgEGMAsGCSqGSIb3DQEBCwOCAQEAaK6K7/0Y+PkG\n"
    "MQJjumTlt6XUQjQ3Y6zuSOMlZ1wmJoBqWabYhJ4qXfcSMQiw+kZ+mQTFk+IdurGC\n"
    "RHrAKwDGNRqmjnQ56qjwHNTTxhJozP09vBCgs3fIQQY/Nq/uISoQvOZmoIriFZf6\n"
    "8czHMlj1vIC6zp4XHWdqkQ7aF4YFsTfM0kBPrm0Y6Nn0VKzWNdmaIs/X5OcR6ZAG\n"
    "zGN9UZV+ZftcfdqI0XSVCVRAK5MeEa+twLr5PE/Nl7/Ig/tUJMWGSbcrWRZQTXQu\n"
    "Rx7CSKcoatyMhJOd2YT4BvoijEJCxTKWMJzFe2uZAphQHUlVmE9IbUQM0T1N6RNd\n"
    "1dH4o4UeuQ==\n"
    "-----END CERTIFICATE-----";

const uint8_t kCaSecondIntermediateDer[] = {
    0x30, 0x82, 0x02, 0xD3, 0x30, 0x82, 0x01, 0xBD, 0xA0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x01, 0x03, 0x30, 0x0B, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86,
    0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x30, 0x1A, 0x31, 0x18, 0x30, 0x16, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0C, 0x0F, 0x63, 0x61, 0x2D, 0x69, 0x6E, 0x74,
    0x65, 0x72, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x74, 0x65, 0x30, 0x22, 0x18,
    0x0F, 0x32, 0x30, 0x31, 0x35, 0x31, 0x31, 0x32, 0x38, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x5A, 0x18, 0x0F, 0x32, 0x30, 0x31, 0x38, 0x30, 0x32,
    0x30, 0x35, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5A, 0x30, 0x21, 0x31,
    0x1F, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x16, 0x63, 0x61,
    0x2D, 0x73, 0x65, 0x63, 0x6F, 0x6E, 0x64, 0x2D, 0x69, 0x6E, 0x74, 0x65,
    0x72, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x74, 0x65, 0x30, 0x82, 0x01, 0x22,
    0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
    0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F, 0x00, 0x30, 0x82, 0x01, 0x0A,
    0x02, 0x82, 0x01, 0x01, 0x00, 0xBA, 0x88, 0x51, 0xA8, 0x44, 0x8E, 0x16,
    0xD6, 0x41, 0xFD, 0x6E, 0xB6, 0x88, 0x06, 0x36, 0x10, 0x3D, 0x3C, 0x13,
    0xD9, 0xEA, 0xE4, 0x35, 0x4A, 0xB4, 0xEC, 0xF5, 0x68, 0x57, 0x6C, 0x24,
    0x7B, 0xC1, 0xC7, 0x25, 0xA8, 0xE0, 0xD8, 0x1F, 0xBD, 0xB1, 0x9C, 0x06,
    0x9B, 0x6E, 0x1A, 0x86, 0xF2, 0x6B, 0xE2, 0xAF, 0x5A, 0x75, 0x6B, 0x6A,
    0x64, 0x71, 0x08, 0x7A, 0xA5, 0x5A, 0xA7, 0x45, 0x87, 0xF7, 0x1C, 0xD5,
    0x24, 0x9C, 0x02, 0x7E, 0xCD, 0x43, 0xFC, 0x1E, 0x69, 0xD0, 0x38, 0x20,
    0x29, 0x93, 0xAB, 0x20, 0xC3, 0x49, 0xE4, 0xDB, 0xB9, 0x4C, 0xC2, 0x6B,
    0x6C, 0x0E, 0xED, 0x15, 0x82, 0x0F, 0xF1, 0x7E, 0xAD, 0x69, 0x1A, 0xB1,
    0xD3, 0x02, 0x3A, 0x8B, 0x2A, 0x41, 0xEE, 0xA7, 0x70, 0xE0, 0x0F, 0x0D,
    0x8D, 0xFD, 0x66, 0x0B, 0x2B, 0xB0, 0x24, 0x92, 0xA4, 0x7D, 0xB9, 0x88,
    0x61, 0x79, 0x90, 0xB1, 0x57, 0x90, 0x3D, 0xD2, 0x3B, 0xC5, 0xE0, 0xB8,
    0x48, 0x1F, 0xA8, 0x37, 0xD3, 0x88, 0x43, 0xEF, 0x27, 0x16, 0xD8, 0x55,
    0xB7, 0x66, 0x5A, 0xAA, 0x7E, 0x02, 0x90, 0x2F, 0x3A, 0x7B, 0x10, 0x80,
    0x06, 0x24, 0xCC, 0x1C, 0x6C, 0x97, 0xAD, 0x96, 0x61, 0x5B, 0xB7, 0xE2,
    0x96, 0x12, 0xC0, 0x75, 0x31, 0xA3, 0x0C, 0x91, 0xDD, 0xB4, 0xCA, 0xF7,
    0xFC, 0xAD, 0x1D, 0x25, 0xD3, 0x09, 0xEF, 0xB9, 0x17, 0x0E, 0xA7, 0x68,
    0xE1, 0xB3, 0x7B, 0x2F, 0x22, 0x6F, 0x69, 0xE3, 0xB4, 0x8A, 0x95, 0x61,
    0x1D, 0xEE, 0x26, 0xD6, 0x25, 0x9D, 0xAB, 0x91, 0x08, 0x4E, 0x36, 0xCB,
    0x1C, 0x24, 0x04, 0x2C, 0xBF, 0x16, 0x8B, 0x2F, 0xE5, 0xF1, 0x8F, 0x99,
    0x17, 0x31, 0xB8, 0xB3, 0xFE, 0x49, 0x23, 0xFA, 0x72, 0x51, 0xC4, 0x31,
    0xD5, 0x03, 0xAC, 0xDA, 0x18, 0x0A, 0x35, 0xED, 0x8D, 0x02, 0x03, 0x01,
    0x00, 0x01, 0xA3, 0x1D, 0x30, 0x1B, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x1D,
    0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF, 0x30, 0x0B, 0x06, 0x03,
    0x55, 0x1D, 0x0F, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x0B, 0x06,
    0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x03, 0x82,
    0x01, 0x01, 0x00, 0x68, 0xAE, 0x8A, 0xEF, 0xFD, 0x18, 0xF8, 0xF9, 0x06,
    0x31, 0x02, 0x63, 0xBA, 0x64, 0xE5, 0xB7, 0xA5, 0xD4, 0x42, 0x34, 0x37,
    0x63, 0xAC, 0xEE, 0x48, 0xE3, 0x25, 0x67, 0x5C, 0x26, 0x26, 0x80, 0x6A,
    0x59, 0xA6, 0xD8, 0x84, 0x9E, 0x2A, 0x5D, 0xF7, 0x12, 0x31, 0x08, 0xB0,
    0xFA, 0x46, 0x7E, 0x99, 0x04, 0xC5, 0x93, 0xE2, 0x1D, 0xBA, 0xB1, 0x82,
    0x44, 0x7A, 0xC0, 0x2B, 0x00, 0xC6, 0x35, 0x1A, 0xA6, 0x8E, 0x74, 0x39,
    0xEA, 0xA8, 0xF0, 0x1C, 0xD4, 0xD3, 0xC6, 0x12, 0x68, 0xCC, 0xFD, 0x3D,
    0xBC, 0x10, 0xA0, 0xB3, 0x77, 0xC8, 0x41, 0x06, 0x3F, 0x36, 0xAF, 0xEE,
    0x21, 0x2A, 0x10, 0xBC, 0xE6, 0x66, 0xA0, 0x8A, 0xE2, 0x15, 0x97, 0xFA,
    0xF1, 0xCC, 0xC7, 0x32, 0x58, 0xF5, 0xBC, 0x80, 0xBA, 0xCE, 0x9E, 0x17,
    0x1D, 0x67, 0x6A, 0x91, 0x0E, 0xDA, 0x17, 0x86, 0x05, 0xB1, 0x37, 0xCC,
    0xD2, 0x40, 0x4F, 0xAE, 0x6D, 0x18, 0xE8, 0xD9, 0xF4, 0x54, 0xAC, 0xD6,
    0x35, 0xD9, 0x9A, 0x22, 0xCF, 0xD7, 0xE4, 0xE7, 0x11, 0xE9, 0x90, 0x06,
    0xCC, 0x63, 0x7D, 0x51, 0x95, 0x7E, 0x65, 0xFB, 0x5C, 0x7D, 0xDA, 0x88,
    0xD1, 0x74, 0x95, 0x09, 0x54, 0x40, 0x2B, 0x93, 0x1E, 0x11, 0xAF, 0xAD,
    0xC0, 0xBA, 0xF9, 0x3C, 0x4F, 0xCD, 0x97, 0xBF, 0xC8, 0x83, 0xFB, 0x54,
    0x24, 0xC5, 0x86, 0x49, 0xB7, 0x2B, 0x59, 0x16, 0x50, 0x4D, 0x74, 0x2E,
    0x47, 0x1E, 0xC2, 0x48, 0xA7, 0x28, 0x6A, 0xDC, 0x8C, 0x84, 0x93, 0x9D,
    0xD9, 0x84, 0xF8, 0x06, 0xFA, 0x22, 0x8C, 0x42, 0x42, 0xC5, 0x32, 0x96,
    0x30, 0x9C, 0xC5, 0x7B, 0x6B, 0x99, 0x02, 0x98, 0x50, 0x1D, 0x49, 0x55,
    0x98, 0x4F, 0x48, 0x6D, 0x44, 0x0C, 0xD1, 0x3D, 0x4D, 0xE9, 0x13, 0x5D,
    0xD5, 0xD1, 0xF8, 0xA3, 0x85, 0x1E, 0xB9};

// certspec (for pycert.py)
//
// issuer:ca-second-intermediate
// subject:ee
const char* kEePem =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICujCCAaSgAwIBAgIUMy8NE67P/4jkaCra7rOVVvX4+GswCwYJKoZIhvcNAQEL\n"
    "MCExHzAdBgNVBAMMFmNhLXNlY29uZC1pbnRlcm1lZGlhdGUwIhgPMjAxNTExMjgw\n"
    "MDAwMDBaGA8yMDE4MDIwNTAwMDAwMFowDTELMAkGA1UEAwwCZWUwggEiMA0GCSqG\n"
    "SIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6iFGoRI4W1kH9braIBjYQPTwT2erkNUq0\n"
    "7PVoV2wke8HHJajg2B+9sZwGm24ahvJr4q9adWtqZHEIeqVap0WH9xzVJJwCfs1D\n"
    "/B5p0DggKZOrIMNJ5Nu5TMJrbA7tFYIP8X6taRqx0wI6iypB7qdw4A8Njf1mCyuw\n"
    "JJKkfbmIYXmQsVeQPdI7xeC4SB+oN9OIQ+8nFthVt2Zaqn4CkC86exCABiTMHGyX\n"
    "rZZhW7filhLAdTGjDJHdtMr3/K0dJdMJ77kXDqdo4bN7LyJvaeO0ipVhHe4m1iWd\n"
    "q5EITjbLHCQELL8Wiy/l8Y+ZFzG4s/5JI/pyUcQx1QOs2hgKNe2NAgMBAAEwCwYJ\n"
    "KoZIhvcNAQELA4IBAQCE5V5YiFPtbb1dOCIMGC5X/6kfQkQmIfvEZIol0MRXmP4g\n"
    "CsOPbTI+BNxYVNk5RHIlr+6e0d8TNiABem4FZK3kea4ugN8ez3IsK7ug7qdrooNA\n"
    "MiHOvrLmAw2nQWexdDRf7OPeVj03BwELzGTOGPjAqDktTsK57OfXyFTm9nl75WQo\n"
    "+EWX+CdV4L1o2rgABvSiMnMdycftCC73Hr/3ypADqY7nDrKpxYdrGgzAQvx3DjPv\n"
    "b7nBKH/gXg3kzoWpeQmJYPl9Vd+DvGljS5i71oLbvCwlDX7ZswGcvb8pQ7Tni5HA\n"
    "VYpAYLokxIDFnyVT9oCACJuJ5LvpBBrhd0+1uUPE\n"
    "-----END CERTIFICATE-----";

class psm_CertList : public ::testing::Test {
 protected:
  void SetUp() override {
    nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
    ASSERT_TRUE(prefs)
    << "couldn't get nsIPrefBranch";

    // When PSM initializes, it attempts to get some localized strings.
    // As a result, Android flips out if this isn't set.
    nsresult rv = prefs->SetBoolPref("intl.locale.matchOS", true);
    ASSERT_TRUE(NS_SUCCEEDED(rv))
    << "couldn't set pref 'intl.locale.matchOS'";

    nsCOMPtr<nsIX509CertDB> certdb(do_GetService(NS_X509CERTDB_CONTRACTID));
    ASSERT_TRUE(certdb)
    << "couldn't get certdb";
  }
};

static nsresult AddCertFromStringToList(
    const char* aPem, nsTArray<RefPtr<nsIX509Cert>>& aCertList) {
  RefPtr<nsIX509Cert> cert;
  cert =
      nsNSSCertificate::ConstructFromDER(const_cast<char*>(aPem), strlen(aPem));
  if (!cert) {
    return NS_ERROR_FAILURE;
  }

  aCertList.AppendElement(cert);
  return NS_OK;
}

TEST_F(psm_CertList, TestInvalidSegmenting) {
  nsTArray<RefPtr<nsIX509Cert>> certList;

  nsTArray<nsTArray<uint8_t>> intCerts;
  nsresult rv = nsNSSCertificate::GetIntermediatesAsDER(certList, intCerts);
  ASSERT_EQ(rv, NS_ERROR_INVALID_ARG) << "Empty lists can't be segmented";

  rv = AddCertFromStringToList(kCaPem, certList);
  ASSERT_EQ(rv, NS_OK) << "Should have loaded OK";

  intCerts.Clear();

  rv = nsNSSCertificate::GetIntermediatesAsDER(certList, intCerts);
  ASSERT_EQ(rv, NS_ERROR_INVALID_ARG) << "Lists of one can't be segmented";
}

TEST_F(psm_CertList, TestValidSegmenting) {
  nsTArray<RefPtr<nsIX509Cert>> certList;

  nsresult rv = AddCertFromStringToList(kEePem, certList);
  ASSERT_EQ(rv, NS_OK) << "Should have loaded OK";
  rv = AddCertFromStringToList(kCaSecondIntermediatePem, certList);
  ASSERT_EQ(rv, NS_OK) << "Should have loaded OK";

  nsTArray<nsTArray<uint8_t>> intCerts;

  rv = nsNSSCertificate::GetIntermediatesAsDER(certList, intCerts);
  ASSERT_EQ(rv, NS_OK) << "Should have segmented OK";
  ASSERT_EQ(intCerts.Length(), static_cast<size_t>(0))
      << "There should be no intermediates";

  rv = AddCertFromStringToList(kCaIntermediatePem, certList);
  ASSERT_EQ(rv, NS_OK) << "Should have loaded OK";

  intCerts.Clear();
  rv = nsNSSCertificate::GetIntermediatesAsDER(certList, intCerts);
  ASSERT_EQ(rv, NS_OK) << "Should have segmented OK";

  ASSERT_EQ(intCerts.Length(), static_cast<size_t>(1))
      << "There should be one intermediate";

  rv = AddCertFromStringToList(kCaPem, certList);
  ASSERT_EQ(rv, NS_OK) << "Should have loaded OK";

  intCerts.Clear();
  rv = nsNSSCertificate::GetIntermediatesAsDER(certList, intCerts);
  ASSERT_EQ(rv, NS_OK) << "Should have segmented OK";

  ASSERT_EQ(intCerts.Length(), static_cast<size_t>(2))
      << "There should be two intermediates";

  const nsTArray<uint8_t> secondIntermediateBinaryArray(
      kCaSecondIntermediateDer, sizeof(kCaSecondIntermediateDer));
  const nsTArray<uint8_t> intermediateBinaryArray(kCaIntermediateDer,
                                                  sizeof(kCaIntermediateDer));

  for (size_t i = 0; i < intCerts.Length(); ++i) {
    const nsTArray<uint8_t>& certArray = intCerts[i];

    if (i < intCerts.Length() - 1) {
      if (certArray != secondIntermediateBinaryArray) {
        rv = NS_ERROR_FAILURE;
        break;
      }
    } else {
      if (certArray != intermediateBinaryArray) {
        rv = NS_ERROR_FAILURE;
        break;
      }
    }
  }

  ASSERT_EQ(rv, NS_OK) << "Should have looped OK.";
}

TEST_F(psm_CertList, TestGetRootCertificateChainTwo) {
  nsTArray<RefPtr<nsIX509Cert>> certList;

  nsresult rv = AddCertFromStringToList(kCaIntermediatePem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";
  rv = AddCertFromStringToList(kCaPem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";

  nsCOMPtr<nsIX509Cert> rootCert;
  rv = nsNSSCertificate::GetRootCertificate(certList, rootCert);
  EXPECT_EQ(NS_OK, rv) << "Should have fetched the root OK";
  ASSERT_TRUE(rootCert)
  << "Root cert should be filled in";

  nsAutoString rootCn;
  EXPECT_TRUE(NS_SUCCEEDED(rootCert->GetCommonName(rootCn)))
      << "Getters should work.";
  EXPECT_TRUE(rootCn.EqualsLiteral("ca")) << "Root CN should match";

  // Re-fetch and ensure we get the same certificate.
  nsCOMPtr<nsIX509Cert> rootCertRepeat;
  rv = nsNSSCertificate::GetRootCertificate(certList, rootCertRepeat);
  EXPECT_EQ(NS_OK, rv) << "Should have fetched the root OK the second time";
  ASSERT_TRUE(rootCertRepeat)
  << "Root cert should still be filled in";

  nsAutoString rootRepeatCn;
  EXPECT_TRUE(NS_SUCCEEDED(rootCertRepeat->GetCommonName(rootRepeatCn)))
      << "Getters should work.";
  EXPECT_TRUE(rootRepeatCn.EqualsLiteral("ca")) << "Root CN should still match";
}

TEST_F(psm_CertList, TestGetRootCertificateChainFour) {
  nsTArray<RefPtr<nsIX509Cert>> certList;

  nsresult rv = AddCertFromStringToList(kEePem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";
  rv = AddCertFromStringToList(kCaSecondIntermediatePem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";
  rv = AddCertFromStringToList(kCaIntermediatePem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";
  rv = AddCertFromStringToList(kCaPem, certList);
  ASSERT_EQ(NS_OK, rv) << "Should have loaded OK";

  nsCOMPtr<nsIX509Cert> rootCert;
  rv = nsNSSCertificate::GetRootCertificate(certList, rootCert);
  EXPECT_EQ(NS_OK, rv) << "Should have again fetched the root OK";
  ASSERT_TRUE(rootCert)
  << "Root cert should be filled in";

  nsAutoString rootCn;
  EXPECT_TRUE(NS_SUCCEEDED(rootCert->GetCommonName(rootCn)))
      << "Getters should work.";
  EXPECT_TRUE(rootCn.EqualsLiteral("ca")) << "Root CN should match";
}

TEST_F(psm_CertList, TestGetRootCertificateChainEmpty) {
  nsTArray<RefPtr<nsIX509Cert>> certList;

  nsCOMPtr<nsIX509Cert> rootCert;
  nsresult rv = nsNSSCertificate::GetRootCertificate(certList, rootCert);
  EXPECT_EQ(NS_ERROR_FAILURE, rv)
      << "Should have returned NS_ERROR_FAILURE because certList was empty";
  EXPECT_FALSE(rootCert) << "Root cert should be empty";
}
