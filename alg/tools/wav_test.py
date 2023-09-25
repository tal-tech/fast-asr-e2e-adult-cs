import numpy as np
import kaldiio
import kaldi_io
import torchaudio
import torchaudio.compliance.kaldi as kaldi

def build_LFR_features(inputs, m=7, n=6):
    LFR_inputs = []
    T = inputs.shape[0]
    T_lfr = int(np.ceil(T / n))
    #print("T_lfr: ", T_lfr)
    left_padding = np.tile(inputs[0], ((m - 1) // 2, 1))
    #print("left_padding shape: ", left_padding.shape)
    inputs = np.vstack((left_padding, inputs))
    #print("inputs shape: ", inputs.shape)
    T = T + (m - 1) // 2
    for i in range(T_lfr):
        if m <= T - i * n:
            LFR_inputs.append(np.hstack(inputs[i * n:i * n + m]))
        else:
            num_padding = m - (T - i * n)
            frame = np.hstack(inputs[i * n:])
            for _ in range(num_padding):
                frame = np.hstack((frame, inputs[-1]))
            LFR_inputs.append(frame)
    return np.vstack(LFR_inputs)

def build_CMVN_features(inputs, mvn_file):  # noqa
    with open(mvn_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    add_shift_list = []
    rescale_list = []
    for i in range(len(lines)):
        line_item = lines[i].split()
        #print("line_item: ", line_item)
        if line_item[0] == '<AddShift>':
            line_item = lines[i + 1].split()
            if line_item[0] == '<LearnRateCoef>':
                add_shift_line = line_item[3:(len(line_item) - 1)]
                add_shift_list = list(add_shift_line)
                continue
        elif line_item[0] == '<Rescale>':
            line_item = lines[i + 1].split()
            if line_item[0] == '<LearnRateCoef>':
                rescale_line = line_item[3:(len(line_item) - 1)]
                rescale_list = list(rescale_line)
                continue
    
    #np.save("./add_shift.npy", np.array(add_shift_list).astype(np.float32))
    #np.save("./rescale.npy", np.array(rescale_list).astype(np.float32))
    for j in range(inputs.shape[0]):
        for k in range(inputs.shape[1]):
            add_shift_value = add_shift_list[k]
            rescale_value = rescale_list[k]
            inputs[j, k] = float(inputs[j, k]) + float(add_shift_value)
            inputs[j, k] = float(inputs[j, k]) * float(rescale_value)

    return inputs

waveform, sample_rate = torchaudio.load("/home/wangyaoping/wangyaoping/wyp_data/test/Leaderboard/datasets/SPEECHIO_ASR_ZH00001/wav/2dlcYQi51Sk__20190615_CCTV_201.wav")
waveform = waveform * (1 << 15)

mat = kaldi.fbank(waveform,
                      num_mel_bins=80,
                      frame_length=25,
                      frame_shift=10,
                      dither=0.0,
                      energy_floor=0.0,
                      window_type='hamming',
                      sample_frequency=16000)

mat = mat.numpy()

#print("waveform: ", waveform)
#print(waveform.shape)

print("mat: ", mat)
#print(mat.shape)

feature = build_LFR_features(mat)
#print(feature)
feature = build_CMVN_features(feature, "./am.mvn")
#print(feature)
#b = kaldiio.load_ark("/home/wangyaoping/wangyaoping/wyp_exp/ASR/FunASR/egs_modelscope/tal/paraformer/test_data/dump/fbank/aishell_one/ark/feats.1.ark")
#for i,j in b:
#    print(j)
