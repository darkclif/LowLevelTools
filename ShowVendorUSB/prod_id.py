#   Software for searching info about USB devices 
#
#   Author: Maciej Wydro
#
#   Usage:
#       prod_id.py pppp dddd
#       prod_id.py -u

import re
import argparse

# Setup args
parser = argparse.ArgumentParser(description='Search info of USB device.')
parser.add_argument('-v', metavar='VENDOR-ID', dest='vendor', help='ID of vendor (0000-ffff)')
parser.add_argument('-d', metavar='DEVICE-ID', dest='device', help='ID of device (0000-ffff)')
#parser.add_argument('--update', action='store_const', const=True, dest='update', default=False, help='Update database')

args = parser.parse_args()

#################################################
################# Read database #################
#################################################
DB_FILE  = "./usb.ids"
producent_db = {}

print("[INFO] Reading data from %s file" % DB_FILE)

prod_start = re.compile(r'^([0-9a-f]{4})  (.*)$')
prod_item = re.compile(r'^\t([0-9a-f]{4})  (.*)$')

curr_prod = None
line_i = 0
for line in open(DB_FILE, "r"):
    line_i += 1

    # Check if prod_id
    result = prod_start.match(line)
    
    if result:
        #print("Id:", result.group(1), "Name:", result.group(2))

        curr_prod = result.group(1)
        producent_db[result.group(1)] = {"name": result.group(2), "devices": {}}
        continue
        
    # Check if prod_item
    result = prod_item.match(line)

    if result:
        #print("   Id:", result.group(1), "Name:", result.group(2))

        if curr_prod:
            producent_db[curr_prod]["devices"][result.group(1)] = result.group(2)
        else:
            print("[ERROR] Line %d: Device readed before vendor, skipping." % line_i)
        continue

    # Maybe end of data?

print("[INFO] Data readed. Vendors: %d." % len(producent_db.items()))

#################################################
################# Show info #####################
#################################################
print()

def printd(s):
    print("Device name [%s]: %s" % (args.device, s))

def printv(s):
    print("Vendor name [%s]: %s" % (args.vendor, s))

if not args.vendor and not args.device:
    print('You must pass at least vendor ID or device ID. (try -h)')
    exit()

if args.vendor:
    if args.vendor in producent_db.keys():
        l_vendor = producent_db[args.vendor]

        printv(l_vendor["name"])
        if args.device:
            if args.device in l_vendor["devices"]:
                printd(l_vendor["devices"][args.device])
            else:
                printd("Not found.")            
    else:
        printv("Not found.")
else:
    if args.device:
        for vendor in producent_db.values():
            if args.device in vendor["devices"]:
                print("Possible device name [%s]: %s" % (args.device, vendor["devices"][args.device]))
    else:
        print('You must pass at least vendor ID or device ID. (try -h)')
