# lightning-cluster
A set of programs to quickly bootstrap lightning nodes.     
Compile and run ./<cluster> <n>   
  to start n c-lightning nodes and to fund them with 1 Bitcoin each.   
  Then type in a series of numbers to connect and create a channel.   
  For example, 0 1 2 0 creates a cyclic set of channels.   
  Comment out relevant lines of code for automatic channel creation.   
  Run `bitcoin-cli generate 6` to confirm funding transactions.   
  Do `source .clnalias` to get a easy way to refer to your nodes like lnc<node number>   
  
WARNING : expect breaking changes soon, including adding config files and arguments   
          There is no error checking right now.   
          c-lightning supports only one channel for two peers right now.   
          
NOTE - Only c-lightning is supported right now. Eclair support will soon be up.    
       Does not follow best code practices, this repo has an older file. Will clean-up code and upload a better one with more programs over the next couple of days.   
        Run `rm -rf *; killall lightningd` in `~/sim/cln`   
        Before starting up a new set of nodes.    
        
      
