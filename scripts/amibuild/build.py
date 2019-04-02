#!/usr/bin/python3

# build.py is a script that builds an Amazon EC2 AMI with BSG tools/IP
# directories installed and configured. It uses the boto3 library to interact
# with the AWS console. The script launches an instance, passes a UserData
# script, waits for the instance to stop itself, and then creates an AMI.
import boto3
import datetime
import time
import argparse 
import os
import inspect
from functools import reduce
from ReleaseRepoAction import ReleaseRepoAction
from AfiAction import AfiAction

parser = argparse.ArgumentParser(description='Build an AWS EC2 F1 FPGA Image')
parser.add_argument('Release', action=ReleaseRepoAction, nargs=1,
                    help='BSG Release repository for this build as: repo_name@commit_id')
parser.add_argument('AfiId', action=AfiAction, nargs=1,
                    default={"AmazonFpgaImageID":"Not-Specified-During-AMI-Build"},
                    help='JSON File Path with "FpgaImageId" and "FpgaImageGlobalId" defined')
parser.add_argument('-d', '--dryrun', action='store_const', const=True,
                    help='Process the arguments but do not launch an instance')

args = parser.parse_args()

# The timestamp is used in the instance name and the AMI name
timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
instance_name = timestamp + '_image_build'
ami_name = 'BSG AMI ' + timestamp
base_ami = 'ami-093cf634bf32a0b7e'
# The instance type is used to build the image - it does not need to match the
# final instance type (e.g. an F1 instance type)
instance_type = 't2.2xlarge'

# Connect to AWS Servicesn
ec2 = boto3.resource('ec2')
cli = boto3.client('ec2')

# Create a "waiter" to wait on the "Stopped" state
waiter = cli.get_waiter('instance_stopped')

# Open Userdata (bootstrap.init) and pass it the name of the current release repository
curscr = os.path.abspath(inspect.getfile(inspect.currentframe()))
curdir = os.path.dirname(curscr)
bootstrap_path = os.path.join(curdir, "bootstrap.init")

UserData = open(bootstrap_path,'r').read()
UserData = UserData.replace("$release_repo", args.Release["name"])
UserData = UserData.replace("$release_hash", args.Release["commit"])

if(args.dryrun):
    print(UserData)
    exit(1)

# Create and launch an instance
instance = ec2.create_instances(
    ImageId=base_ami,
    InstanceType=instance_type,
    KeyName='cad-xor',
    SecurityGroupIds=['bsg_sg_xor_uswest2'],
        UserData=UserData,
        MinCount=1,
    MaxCount=1,
    TagSpecifications=[{'ResourceType':'instance',
                         'Tags':[{'Key':'Name',
                                  'Value':instance_name}]}],
    BlockDeviceMappings=[
        {
            'DeviceName': '/dev/sda1',
            'Ebs': {
                'DeleteOnTermination': True,
                'VolumeSize': 150,
            }
        },
    ])[0]

print('Generated Instance: ' + instance.id);

# This is necessary to give the instance some time to be registered
instance.wait_until_running()
print("Instance running. Waiting for instance to enter 'Stopped' state.")
waiter.wait(
    InstanceIds=[
        instance.id,
    ],
    WaiterConfig={
        'Delay': 60,
        'MaxAttempts': 180
    }
)
print('Instance configuration completed')

# Finally, generate the AMI 
ami = cli.create_image(InstanceId=instance.id, Name=ami_name, 
                       Description="BSG AMI with release repository {}@{}".format(args.Release["name"], args.Release["commit"]))
print('Creating AMI: ' + ami['ImageId'])
