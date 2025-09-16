# Launch llama server with a model.

#sudo cpupower frequency-set  --governor performance
#cpupower frequency-info -o proc

/home/radxa/Dev/llama.cpp/build/bin/llama-server \
  -m /home/radxa/Dev/llamacpp_models/unsloth_Qwen3-4B-Instruct-2507-GGUF_Qwen3-4B-Instruct-2507-Q8_0.gguf \
  --threads 8 --temp 0.7 --top-p 0.8 --min-p 0.0 --top-k 20 \
  --presence-penalty 0.15 \
  --cpu-range 5-11 \
  --host 192.168.1.105 \
  --port 8080 --api-key local --jinja --mlock

#sudo cpupower frequency-set --governor schedutil
#cpupower frequency-info -o proc
