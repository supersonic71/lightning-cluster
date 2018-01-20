# lightning-cluster
A set of programs to quickly bootstrap lightning nodes. 

Compile and run ./<cluster> <n>   
  to start n c-lightning nodes and to fund them with 1 Bitcoin each.   
   
  Run `bitcoin-cli generate 6` to confirm funding transactions.   
  Do `source .clnalias` to get a easy way to refer to your nodes like lnc<node number>   
  
WARNING : expect breaking changes soon, including adding config files and arguments   
          There is no error checking right now.   
          c-lightning supports only one channel for a peer right now.   
          
NOTE - Only c-lightning is supported right now. Eclair support will soon be up.    
        Run `rm -rf *; killall lightningd` in `node_path`   
        Before starting up a new set of nodes.    
        
      
