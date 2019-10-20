#!/bin/sh
openssl genrsa -out pr.pem 256
openssl rsa -in pr.pem -out pr.der -outform DER
openssl rsa -in pr.pem -pubout -out pu.der -outform DER
rm pr.pem
