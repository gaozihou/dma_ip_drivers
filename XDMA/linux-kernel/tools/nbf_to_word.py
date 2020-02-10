
import sys
import math

if __name__ == "__main__":

  if len(sys.argv) == 2:
    filename = sys.argv[1]
    
    # read nbf file
    f = open(filename, "r")
    lines = f.readlines()
    
    # define constant
    addr_width = 40
    block_size = 64
    opcode_width = 8
    total_width = addr_width + block_size + opcode_width
    
    # number of words to be sent for each line
    word_width = 32
    num_words = int(math.ceil(total_width / (word_width*1.0)))
    padded_width = word_width * num_words
    
    for line in lines:
      # format line
      line = line.replace("_", "")
      line = line.replace("\n", "")
      line = line.zfill(num_words * word_width / 4)
      
      # write word
      for i in range(num_words):
        word = line[(word_width*num_words/4-(i+1)*word_width/4):(word_width*num_words/4-i*word_width/4)]
        print(word)
    
  else:
    print("USAGE:")
    command = "python nbf_to_word.py {program.nbf}"
    print(command)
