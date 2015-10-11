#!/bin/sh

# Script to build the Fremantle package for PortaBase from a Mac
# Run this from the project root, like packaging/maemo/fremantle_vm.sh

VERSION=`cat packaging/version_number`
FREMANTLE_DIR=build/fremantle
DIR_NAME=portabase-$VERSION
DEST=$FREMANTLE_DIR/$DIR_NAME
TARBALL=$DIR_NAME.tar.gz
VM_NAME="Fremantle SDK"
SSH_PORT=3023
SCRATCHBOX_HOME=/scratchbox/users/maemo/home/maemo/

# Prepare the source code tarball, including translated help files
rm -rf $FREMANTLE_DIR
packaging/copy_source.sh $DEST
# Add "maemo5" to CONFIG in portabase.pro
sed -i '' 's/warn_on/warn_on maemo5/g' $DEST/portabase.pro
cd $DEST
packaging/maemo/generate_help.sh
cd ..
tar -czf $TARBALL $DIR_NAME

# Start the VM containing the Fremantle SDK and copy the code into it.
# Assumes that:
#  * /home/maemo/.ssh/authorized_keys in the VM includes your ssh public key
#  * you've enabled port mapping in VirtualBox to allow ssh into the VM
VBoxManage startvm "$VM_NAME"
ATTEMPT=1
until scp -P $SSH_PORT $TARBALL maemo@127.0.0.1:$SCRATCHBOX_HOME
do
    if [ $ATTEMPT -gt 20 ]
    then
	exit
    fi
    sleep 5
    ATTEMPT+=1
done

# ssh into the VM, log into scratchbox, and build the PortaBase package
ssh -p $SSH_PORT maemo@127.0.0.1 << VM
/scratchbox/login - << SCRATCHBOX
sb-conf se FREMANTLE_ARMEL
rm -rf $DIR_NAME
tar xzf $TARBALL
rm $TARBALL
cd $DIR_NAME
packaging/maemo/fremantle.sh
rm -rf $FREMANTLE_DIR/$DIR_NAME
SCRATCHBOX
VM

# Copy the generated package out of the VM
scp -r -P $SSH_PORT maemo@127.0.0.1:$SCRATCHBOX_HOME$DIR_NAME/$FREMANTLE_DIR build/

# Shut off the VM
VBoxManage controlvm "$VM_NAME" poweroff
