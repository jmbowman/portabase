#!/bin/sh

# Script to build the Diablo package for PortaBase from a Mac
# Run this from the project root, like packaging/maemo/diablo_vm.sh

VERSION=`cat packaging/version_number`
DIABLO_DIR=build/diablo
DIR_NAME=portabase-$VERSION
DEST=$DIABLO_DIR/$DIR_NAME
TARBALL=$DIR_NAME.tar.gz
VM_NAME="Diablo SDK"
SSH_PORT=3022
SCRATCHBOX_HOME=/scratchbox/users/maemo/home/maemo/

# Prepare the source code tarball, including translated help files
rm -rf $DIABLO_DIR
packaging/copy_source.sh $DEST
cd $DEST
packaging/maemo/generate_help.sh
cd ..
tar -czf $TARBALL $DIR_NAME

# Start the VM containing the Diablo SDK and copy the code into it.
# Assumes that:
#  * /home/maemo/.ssh/authorized_keys in the VM includes your ssh public key
#  * you've enabled port mapping in VirtualBox to allow ssh into the VM
VBoxManage startvm "$VM_NAME"
ATTEMPT=1
until scp -P $SSH_PORT $TARBALL maemo@127.0.0.1:$SCRATCHBOX_HOME
do
    echo "Hello???"
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
sb-conf se DIABLO_ARMEL
rm -rf $DIR_NAME
tar xzf $TARBALL
rm $TARBALL
cd $DIR_NAME
packaging/maemo/diablo.sh
rm -rf $DIABLO_DIR/$DIR_NAME
SCRATCHBOX
VM

# Copy the generated package out of the VM
scp -r -P $SSH_PORT maemo@127.0.0.1:$SCRATCHBOX_HOME$DIR_NAME/$DIABLO_DIR build/

# Shut off the VM
VBoxManage controlvm "$VM_NAME" poweroff
