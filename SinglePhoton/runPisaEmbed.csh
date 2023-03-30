#!/bin/sh

INPUT=$(( $1 ))
START=$(( $1 )) 
source /opt/phenix/core/bin/phenix_setup.csh


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/gpfs/mnt/gpfs02/phenix/plhf/plhf3/nnovitzk/mazsi_Test/ccnt/source/emc-evaluation/build/.libs/:/direct/phenix+u/nivram/myinstall
export INSTALL=/phenix/plhf/nivram/myinstall
export LD_LIBRARY_PATH=$INSTALL/lib:$LD_LIBRARY_PATH
export TSEARCHPATH=/direct/phenix+u/nivram/myinstall
#export DCACHE_DOOR=phnxcore03.rcf.bnl.gov:1094
#export GSEARCHPATH ${GSEARCHPATH}:DCACHE

echo $LD_LIBRARY_PATH

chmod g+rx ${_CONDOR_SCRATCH_DIR}
cd ${_CONDOR_SCRATCH_DIR}
base="/gpfs/mnt/gpfs02/phenix/plhf/plhf1/nivram/Simulation"

echo "==============================================="
echo "============= START SIMULATION  ==============="
echo "==============================================="


# select the CNT file for embeddding
CNTin=`cat $base/common/RealFileListdAu.txt`

NUM=0
SHUF=`shuf -i1-2655 -n1`
for icnt in $CNTin
do
  if [ $NUM == $SHUF ]
  then
     CntFile=$icnt
  fi
  NUM=$(( $NUM + 1 ))
done



CNTFILE=`ls $CntFile | awk -F\/ '{printf("%s\n",$10)}'`
echo $CNTFILE

data=${CntFile:105:62}
runnumber=${CntFile:148:6}
segment=${CntFile:157:2}



#INPUT=$(( $1 +   $2 ))
echo $INPUT



DIR=`printf "%05d" $INPUT`
mkdir -p $DIR
pushd $DIR
echo start the epsilon-f simulation in directory $DIR

PTMIN=0.0
PTMAX=30

cp -r $base/SinglePhoton/make_oscar_photon.C .

root -l -b -q  "make_oscar_photon.C(\"oscar_single_photon.input.$DIR\",\"$base/common/VertexFromData/trimData/VertexSample"$runnumber"-"$segment".txt\")"

echo "==============================================="
echo "============= START PISA NOW =================="
echo "==============================================="
mkdir pisa
pushd pisa

# set up Run16 PISA
mv ../oscar_single_photon.input.$DIR oscar.particles.dat
cp -r $base/common/skel_pisa/* . 
pisa<pisa.input

echo "==============================================="
echo "================ PISA TO DST =================="
echo "==============================================="
MINSIZE=10k # minimum size of pisaToDST output 5MB
root -b -q -l pisaToDST.C
icent=4
cp dst_out.root ../dst_out_photon-$PTMIN-$PTMAX'GeV'-$DIR-$icent.root
#cp dst_out.root  $base/SinglePhoton/embedding_0_30GeV/


#rm dst_out.root
# check the pisaToDST output files
ls -lhtr ../dst_out_photon-*.root

popd
#rm -rf pisa

echo "==============================================="
echo "============= START EMBEDDING NOW ============="
echo "==============================================="
MODE=2 # with embedding
mkdir embedding
pushd embedding


# fetch CNT file from dCache
SEED=$(( $START ))
SLEEP=5
echo $SLEEP
sleep $SLEEP
echo xrdcp root://phnxcore03.rcf.bnl.gov:1094$CntFile .
xrdcp root://phnxcore03.rcf.bnl.gov:1094$CntFile .

# can also insert check here if copy from dCache has a large failure rate
ls -lhtr . 
CNTFILE=`ls $CntFile | awk -F\/ '{printf("%s\n",$10)}'`
echo $CNTFILE

# split CNT file to different centralities
ln -s $base/common/split_realdata.C .
CntFileName=`ls $CntFile | awk -F\/ '{printf("%s\n",$10)}'`
# the output is too large is process through all the events
echo split $CntFileName to different centralities
root -b -q 'split_realdata.C("'$CntFileName'",0,50,200000)'

mkdir cent$icent
ls -lhtr realdst*.root
mv realdst$icent.root cent$icent/realdst.root
mv ../dst_out_photon-$PTMIN-$PTMAX'GeV'-$DIR-$icent.root cent$icent/simdst.root
pushd cent$icent
cp -r $base/common/run_embed.C .
embeddst=`echo embed_photon-$PTMIN-$PTMAX'GeV'-$DIR-$icent.root`
echo generating embeded file $embeddst

root -l -q 'run_embed.C("realdst.root","simdst.root","'$embeddst'", -1, 0)'  >>$base/SinglePhoton/VertexFromData/VertexSample"$runnumber"-"$segment"-"$DIR".txt

cp -r /direct/phenix+u/nivram/myinstall/lib/libsimpphoton.so.0.0.0 . 
cp -r $base/SinglePhoton/simpphoton.C .
ttree=`echo TTree_out_photon-$PTMIN-$PTMAX'GeV'-$DIR-$icent.root`
root -l -b -q 'simpphoton.C("'$embeddst'","'$ttree'",454774,-1,0,4)'

cd ..

mv cent$icent/$ttree .
popd
# rm -rf cent$icent



OutputFileName=`echo TTree_out_photon-$PTMIN-$PTMAX'GeV'-$DIR.root`
hadd $OutputFileName TTree_out_photon-*.root

mv $OutputFileName ..
popd
#rm -rf embedding

echo "==============================================="
echo "=============== END SIMULATION  ==============="
echo "==============================================="

echo embedding to TTree finished in directory $DIR
echo move $OutputFileName to  $base/embedding_0_30GeV/energyScale_1p2up/
mv $OutputFileName  $base/SinglePhoton/embedding_0_30GeV/energyScale_1p2up/



popd
#rm -r $DIR

