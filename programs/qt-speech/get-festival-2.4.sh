#!/bin/sh

DIR=$(dirname "$0")

mkdir -p "${DIR}/festival"
cd "${DIR}/festival"

if [ ! -d "2.4" ]; then
	mkdir -p 2.4
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/speech_tools-2.4-release.tar.gz -o 2.4/speech_tools-2.4-release.tar.gz
	#curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/speech_tools-2.4-with-wrappers.tar.gz -o 2.4/speech_tools-2.4-with-wrappers.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/festival-2.4-release.tar.gz -o 2.4/festival-2.4-release.tar.gz
fi

if [ ! -d "2.4/lexicon" ]; then
	mkdir -p 2.4/lexicon
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/festlex_CMU.tar.gz -o 2.4/lexicon/festlex_CMU.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/festlex_OALD.tar.gz -o 2.4/lexicon/festlex_OALD.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/festlex_POSLEX.tar.gz -o 2.4/lexicon/festlex_POSLEX.tar.gz
fi

if [ ! -d "2.4/voices" ]; then
	mkdir -p 2.4/voices
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_ahw_cg.tar.gz -o 2.4/voices/festvox_cmu_us_ahw_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_aup_cg.tar.gz -o 2.4/voices/festvox_cmu_us_aup_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_awb_cg.tar.gz -o 2.4/voices/festvox_cmu_us_awb_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_axb_cg.tar.gz -o 2.4/voices/festvox_cmu_us_axb_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_bdl_cg.tar.gz -o 2.4/voices/festvox_cmu_us_bdl_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_clb_cg.tar.gz -o 2.4/voices/festvox_cmu_us_clb_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_fem_cg.tar.gz -o 2.4/voices/festvox_cmu_us_fem_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_gka_cg.tar.gz -o 2.4/voices/festvox_cmu_us_gka_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_jmk_cg.tar.gz -o 2.4/voices/festvox_cmu_us_jmk_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_ksp_cg.tar.gz -o 2.4/voices/festvox_cmu_us_ksp_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_rms_cg.tar.gz -o 2.4/voices/festvox_cmu_us_rms_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_rxr_cg.tar.gz -o 2.4/voices/festvox_cmu_us_rxr_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_cmu_us_slt_cg.tar.gz -o 2.4/voices/festvox_cmu_us_slt_cg.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_kallpc16k.tar.gz -o 2.4/voices/festvox_kallpc16k.tar.gz
	curl -C - http://www.cstr.ed.ac.uk/downloads/festival/2.4/voices/festvox_rablpc16k.tar.gz -o 2.4/voices/festvox_rablpc16k.tar.gz
fi

sha1sum -c 2.4-sha1sums || { echo 'sha1sums failed check after downloading' ; exit 1; }

echo "Extracting speech_tools:"
tar -zxf 2.4/speech_tools-2.4-release.tar.gz

#echo "Patching speech_tools for gcc 4.8.2:"
#patch -p1 < 2.4/speech_tools.patch

echo "Building speech_tools:"
cd speech_tools
./configure
make
make install
cd ..

echo "Extracting festival:"
tar -zxf 2.4/festival-2.4-release.tar.gz
echo "Building festival:"
cd festival
./configure
make
cd ..

echo "Extracting lex/vox libraries:"
tar -zxf 2.4/lexicon/festlex_CMU.tar.gz
tar -zxf 2.4/lexicon/festlex_OALD.tar.gz
tar -zxf 2.4/lexicon/festlex_POSLEX.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_ahw_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_aup_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_awb_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_axb_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_bdl_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_clb_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_fem_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_gka_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_jmk_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_ksp_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_rms_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_rxr_cg.tar.gz
tar -zxf 2.4/voices/festvox_cmu_us_slt_cg.tar.gz
tar -zxf 2.4/voices/festvox_kallpc16k.tar.gz
tar -zxf 2.4/voices/festvox_rablpc16k.tar.gz

echo "Testing..."
cd festival
make test

echo '(SayText "Festival is ready")' | bin/festival --pipe

