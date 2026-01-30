import argparse
from python.kws import generate_kws
from python.spectrogram import generate_spectrogram
from python.camera_test import generate_camera_test
from python.generate import generate_common_files

APPS = {'appa':generate_kws, 
    'appb':generate_spectrogram,
    'appc':generate_camera_test}

parser = argparse.ArgumentParser(
                    prog='aidemo',
                    description='Regenerate parts of the demo')

subparsers = parser.add_subparsers(dest="command",required=True,help='subcommand help')

# create the parser for the "a" command
parser_gen = subparsers.add_parser('gen', help='Regenerate an application')
parser_gen.add_argument("--all", help="Regenerate all applications", action='store_true')
parser_gen.add_argument("--size", help="Code size optimization enabled", action='store_true')
parser_gen.add_argument('apps', nargs="*",choices=APPS, help='Application to regenerate')

parser_flash = subparsers.add_parser('flash',help='Regenerate flash memory content')

args = parser.parse_args()

if args.command == "flash":
    print("Not yet implemented")
    exit(0)

if args.all:
    args.apps = APPS
    apps = sorted(APPS.keys())
    for app in apps:
        f = APPS[app]
        print(f"Generating {app}...")
        f(codeSizeOptimization=args.size)
    if args.size:
        print("Regenerating all common files for code size optimization...")
        generate_common_files(all_apps=apps)






