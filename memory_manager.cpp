#include <iostream>
#include <bits/stdc++.h>
using namespace std;

// function used to split the commandlines to vector of strings
vector<string> split(string line)
{
   vector<string> word_vector;
   string word;

   for (char character : line)
   {
      if (character != ' ' && character != ',')
      {
         word += character;
      }
      else if (!word.empty())
      {
         word_vector.push_back(word);
         word.clear();
      }
   }

   if (!word.empty())
   {
      word_vector.push_back(word);
   }

   return word_vector;
}

int M, V, P;           // M V P give for the lab9 executable file only P in bytes
int MainPagesNum;      // Mainpages number
int VirtualPagesNum;   // Virtualpages number
int FreeMainMemory;    // free main memory
int FreeVirtualMemory; // free virtual memory
int pidcount = 1;      // counter added every once a process is loaded in main memory or virtual memory

struct pagetable
{
   int process_size;            // size of the process in KB
   vector<vector<string>> cmds; // stores the commands
   vector<int> vpn_to_pfn;      // stores the vpn to pfn translation
};

map<int, pagetable> pagetablemap;
map<int, int> vmap;
map<int, int> mmap;
vector<int> lru;

int main(int argc, char *argv[])
{
   if (argc != 11)
   {
      cout << "wrong command" << endl;
      return 0;
   }

   M = atoi(argv[2]);
   V = atoi(argv[4]);
   P = atoi(argv[6]);

   string inputfile;
   string outputfile;

   inputfile = argv[8];
   outputfile = argv[10];

   FreeMainMemory = M;
   FreeVirtualMemory = V;
   MainPagesNum = M * 1024 / P;
   VirtualPagesNum = V * 1024 / P;

   vector<int> GiveValue(M * 1024, 0);
   vector<int> vvalue(V * 1024, 0);
   vector<int> FreeMainPages(MainPagesNum, 0);
   vector<int> FreeVirtualPages(VirtualPagesNum, 0);

   ifstream InputFile(inputfile + ".txt"); // assuming .txt is not provided in the commands

   if (InputFile.is_open() == false)
   {
      cerr << inputfile + ".txt"
           << " could not be loaded - file does not exist" << endl;
      return 0;
   }

   ofstream OutputFile(outputfile + ".txt");

   if (OutputFile.is_open() == false)
   {
      cerr << outputfile + ".txt"
           << " could not be loaded - file does not exist" << endl;
      return 0;
   }

   string line;

   while (getline(InputFile, line))
   {
      vector<string> v = split(line);

      if (v[0] == "load")
      {
         int currpid;

         for (int i = 1; i < v.size(); i++)
         {

            currpid = pidcount;

            ifstream loadfile(v[i] + ".txt");

            if (loadfile.is_open() == false)
            {
               OutputFile << v[i] << " could not be loaded - file does not exist" << endl;
               continue;
            }

            string codeline;

            getline(loadfile, codeline);

            int psize = stoi(codeline);
            int ppage = psize * 1024 / P;

            vector<int> pvector(ppage, -1);

            if (psize > FreeMainMemory)
            {
               if (psize > FreeVirtualMemory)
               {
                  OutputFile << v[i] << " could not be loaded - memory is full" << endl;
                  continue;
               }
               else
               {

                  FreeVirtualMemory -= psize;
                  pagetablemap[currpid].process_size = psize;
                  int count = 0;

                  for (int i = 0; i < VirtualPagesNum; i++)
                  {
                     if (FreeVirtualPages[i] == 0)
                     {
                        FreeVirtualPages[i] = 1;
                        pvector[count++] = i;
                     }
                     if (count == ppage)
                     {
                        break;
                     }
                  }

                  pagetablemap[currpid].vpn_to_pfn = pvector;
                  vmap[currpid] = 1;
                  pidcount++;

                  // allocate in vmemory

                  OutputFile << v[i]<< " is loaded in virtual memory and is assigned process id " << currpid << endl;
               }
            }
            else
            {

               pagetablemap[currpid].process_size = psize;
               FreeMainMemory -= psize;

               int count = 0;

               for (int i = 0; i < MainPagesNum; i++)
               {
                  if (FreeMainPages[i] == 0)
                  {
                     FreeMainPages[i] = 1;
                     pvector[count++] = i;
                  }
                  if (count == ppage)
                  {
                     break;
                  }
               }

               pagetablemap[currpid].vpn_to_pfn = pvector;
               mmap[currpid] = 1;
               lru.push_back(currpid);
               pidcount++;

               OutputFile << v[i] << " is loaded in main memory and is assigned process id " << currpid << endl;

               // allocate in mmemory
            }

            while (getline(loadfile, codeline))
            {
               vector<string> codeelem = split(codeline);
               pagetablemap[currpid].cmds.push_back(codeelem);
            }

            loadfile.close();
         }
      }

      if (v[0] == "run")
      {
         int pid = stoi(v[1]);

         if (vmap[pid] == 0 && mmap[pid] == 0)
         {
            OutputFile << "Process doesn't exist" << endl; // process is not in main or virtual memory
            continue;
         }

         if(mmap[pid]==1)
         {
            for(int i=0;i<lru.size();i++)
            {
               if(lru[i]==pid)
               {
                  lru.erase(lru.begin()+i);
                  break;
               }
            }

            lru.push_back(pid);
         }

         if (vmap[pid] == 1)
         {
            int psize = pagetablemap[pid].process_size;

            if (FreeMainMemory >= psize)
            {
               FreeVirtualMemory += psize;

               int count = 0;

               vector<int> tt = pagetablemap[pid].vpn_to_pfn;

               for (int j = 0; j < tt.size(); j++)
               {
                  for (int ii = 0; ii < MainPagesNum; ii++)
                  {
                     if (FreeMainPages[ii] == 0)
                     {
                        FreeMainPages[ii] = 1;

                        for (int k = 0; k < P; k++)
                        {
                           GiveValue[ii * P + k] = vvalue[tt[j] * P + k];
                           vvalue[tt[j] * P + k] = 0;
                        }

                        FreeVirtualPages[tt[j]] = 0;
                        tt[j] = ii;
                        break;
                     }
                  }
               }

               pagetablemap[pid].vpn_to_pfn = tt;
               FreeMainMemory -= psize;
               mmap[pid] = 1;
               vmap[pid] = 0;
               lru.push_back(pid);

               // allocate in main memory
            }

            else
            {
               if (psize > M)
               {
                  OutputFile << "lru failed1" << endl; // very big process can't even fit in main memory
                  continue;
               }
               else
               {

                  int spacealloc = 0;
                  for (int kk = 0; kk < lru.size(); kk++)
                  {
                     spacealloc += pagetablemap[lru[kk]].process_size;
                     if (spacealloc + FreeMainMemory >= psize)
                     {
                        break;
                     }
                  }

                  if (spacealloc > FreeVirtualMemory + psize)
                  {
                     OutputFile << "lru failed2" << endl; // the process removed from main memory cant be fitted in virtual
                     continue;
                  }

                  vmap[pid] = 0;

                  FreeVirtualMemory += psize;

                  int flag = 0;

                  while (FreeMainMemory < psize && lru.size())
                  {

                     int rem_pid = lru[0];

                     if (FreeVirtualMemory >= pagetablemap[rem_pid].process_size)
                     {

                        lru.erase(lru.begin());

                        FreeMainMemory += pagetablemap[rem_pid].process_size;

                        mmap[rem_pid] = 0;

                        vector<int> tt = pagetablemap[rem_pid].vpn_to_pfn;

                        for (int j = 0; j < tt.size(); j++)
                        {
                           for (int jj = 0; jj < VirtualPagesNum; jj++)
                           {
                              if (FreeVirtualPages[jj] == 0)
                              {
                                 FreeVirtualPages[jj] = 1;
                                 for (int k = 0; k < P; k++)
                                 {
                                    vvalue[jj * P + k] = GiveValue[tt[j] * P + k];
                                    GiveValue[tt[j] * P + k] = 0;
                                 }
                                 FreeMainPages[tt[j]] = 0;
                                 tt[j] = jj;
                                 break;
                              }
                           }
                        }

                        pagetablemap[rem_pid].vpn_to_pfn = tt;
                        FreeVirtualMemory -= pagetablemap[rem_pid].process_size;
                        vmap[rem_pid] = 1;
                     }
                     else
                     {
                        flag = 1;
                        break;
                     }
                  }

                  if (flag == 1)
                  {
                     OutputFile << "lru failed3" << endl; // same as failed3 error
                     continue;
                  }

                  vector<int> tt = pagetablemap[pid].vpn_to_pfn;

                  for (int j = 0; j < tt.size(); j++)
                  {
                     for (int ii = 0; ii < MainPagesNum; ii++)
                     {
                        if (FreeMainPages[ii] == 0)
                        {
                           FreeMainPages[ii] = 1;

                           for (int k = 0; k < P; k++)
                           {
                              GiveValue[ii * P + k] = vvalue[tt[j] * P + k];
                              vvalue[tt[j] * P + k] = 0;
                           }

                           FreeVirtualPages[tt[j]] = 0;
                           tt[j] = ii;
                           break;
                        }
                     }
                  }

                  pagetablemap[pid].vpn_to_pfn = tt;
                  FreeMainMemory -= psize;
                  mmap[pid] = 1;
                  lru.push_back(pid);
               }
            }
         }

         

         vector<vector<string>> commands = pagetablemap[pid].cmds; // getting the commands

         for (int i = 0; i < commands.size(); i++)
         {
            if (commands[i][0] == "load")
            {

               int num = stoi(commands[i][1]);
               int vaddr = stoi(commands[i][2]);

               if (vaddr >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr << " specified for process id " << pid << endl;
                  break;
               }

               int vpn = vaddr / P;
               int offset = vaddr - vpn * P;

               int phyaddr = pagetablemap[pid].vpn_to_pfn[vpn] * P + offset;

               GiveValue[phyaddr] = num;

               OutputFile << "Command: load " << num << " , " << vaddr << "; Result: Value of " << num << " is now stored in addr " << vaddr << endl;
            }

            if (commands[i][0] == "add")
            {
               int vaddr1 = stoi(commands[i][1]);
               int vaddr2 = stoi(commands[i][2]);
               int vaddr3 = stoi(commands[i][3]);
               if (vaddr1 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr1 << " specified for process id" << pid << endl; // checking if the virtual address are in bounds
                  break;
               }

               if (vaddr2 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr2 << " specified for process id" << pid << endl;
                  break;
               }

               if (vaddr3 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr3 << " specified for process id" << pid << endl;
                  break;
               }

               int vpn1 = vaddr1 / P;
               int offset1 = vaddr1 - vpn1 * P;
               int phyaddr1 = pagetablemap[pid].vpn_to_pfn[vpn1] * P + offset1;

               int vpn2 = vaddr2 / P;
               int offset2 = vaddr2 - vpn2 * P;
               int phyaddr2 = pagetablemap[pid].vpn_to_pfn[vpn2] * P + offset2;

               int vpn3 = vaddr3 / P;
               int offset3 = vaddr3 - vpn3 * P;
               int phyaddr3 = pagetablemap[pid].vpn_to_pfn[vpn3] * P + offset3;

               GiveValue[phyaddr3] = GiveValue[phyaddr2] + GiveValue[phyaddr1];

               int x = GiveValue[phyaddr1];
               int y = GiveValue[phyaddr2];
               int z = GiveValue[phyaddr3];

               OutputFile << "Commmand: add " << vaddr1 << " , " << vaddr2 << " , " << vaddr3 
                          << "; "
                          << "Result: Value in addr " << vaddr1 << " = " << x << ", addr " << vaddr2 << " = " << y << ", addr " << vaddr3 << " = " << z << endl;
            }

            if (commands[i][0] == "sub")
            {
               int vaddr1 = stoi(commands[i][1]);
               int vaddr2 = stoi(commands[i][2]);
               int vaddr3 = stoi(commands[i][3]);

               if (vaddr1 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr1 << " specified for process id " << pid << endl;
                  break;
               }

               if (vaddr2 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr2 << " specified for process id " << pid << endl;
                  break;
               }

               if (vaddr3 >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr3 << " specified for process id " << pid << endl;
                  break;
               }

               int vpn1 = vaddr1 / P;
               int offset1 = vaddr1 - vpn1 * P;
               int phyaddr1 = pagetablemap[pid].vpn_to_pfn[vpn1] * P + offset1;

               int vpn2 = vaddr2 / P;
               int offset2 = vaddr2 - vpn2 * P;
               int phyaddr2 = pagetablemap[pid].vpn_to_pfn[vpn2] * P + offset2;

               int vpn3 = vaddr3 / P;
               int offset3 = vaddr3 - vpn3 * P;
               int phyaddr3 = pagetablemap[pid].vpn_to_pfn[vpn3] * P + offset3;

               GiveValue[phyaddr3] = GiveValue[phyaddr1] - GiveValue[phyaddr2];

               int x = GiveValue[phyaddr1];
               int y = GiveValue[phyaddr2];
               int z = GiveValue[phyaddr3];

               OutputFile << "Commmand: sub " << vaddr1 << " , " << vaddr2 << " , " << vaddr3 
                          << "; "
                          << "Result: Value in addr " << vaddr1 << " = " << x << ", addr " << vaddr2 << " = " << y << ", addr " << vaddr3 << " = " << z << endl;
            }

            if (commands[i][0] == "print")
            {
               int vaddr = stoi(commands[i][1]);

               if (vaddr >= pagetablemap[pid].process_size * 1024)
               {
                  OutputFile << "Invalid Memory Address " << vaddr << " specified for process id " << pid << endl;
                  break;
               }

               int vpn = vaddr / P;
               int offset = vaddr - vpn * P;

               int phyaddr = pagetablemap[pid].vpn_to_pfn[vpn] * P + offset;

               OutputFile << "Command: print " << vaddr << " ; Result: Value in addr x = " << GiveValue[phyaddr] << endl;
            }
         }
      }

      if (v[0] == "kill")
      {
         int pid = stoi(v[1]);

         if (mmap[pid] == 1)
         {
            FreeMainMemory += pagetablemap[pid].process_size;
            mmap[pid] = 0;

            for (auto tt : pagetablemap[pid].vpn_to_pfn)
            {
               FreeMainPages[tt] = 0;
            }

            pagetablemap[pid].vpn_to_pfn.clear();

            for (int j = 0; j < lru.size(); j++)
            {
               if (lru[j] == pid)
               {
                  lru.erase(lru.begin() + j); // erase after kill
                  break;
               }
            }

            OutputFile << "killed: " << pid << " in main memory" << endl;
         }
         else if (vmap[pid] == 1)
         {
            FreeVirtualMemory += pagetablemap[pid].process_size;
            vmap[pid] = 0;

            for (auto tt : pagetablemap[pid].vpn_to_pfn)
            {
               FreeVirtualPages[tt] = 0;
            }
            pagetablemap[pid].vpn_to_pfn.clear();

            OutputFile << "killed: " << pid << " in virtual memory" << endl;
         }
         else
         {
            OutputFile << "no process to kill" << endl;
         }
      }

      if (v[0] == "listpr")
      {
         vector<int> mainvector;
         vector<int> virvector;
         for (int i = 1; i < pidcount; i++)
         {
            if (mmap[i] == 1)
            {
               mainvector.push_back(i);
            }

            if (vmap[i] == 1)
            {
               virvector.push_back(i);
            }
         }

         sort(mainvector.begin(), mainvector.end());
         sort(virvector.begin(), virvector.end());

         OutputFile << "main memory: ";
         for (auto tt : mainvector)
         {
            OutputFile << tt << " ";
         }
         OutputFile << " |  virtual memory: ";
         for (auto tt : virvector)
         {
            OutputFile << tt << " ";
         }
         OutputFile << endl;
      }

      if (v[0] == "pte")
      {

         int pid = stoi(v[1]);
         string ptefile = v[2];

         ofstream PteFile(ptefile + ".txt", std::ios::app);

         time_t currentTime;
         time(&currentTime);
         tm *localTime = std::localtime(&currentTime);
         char dateTimeString[100];
         strftime(dateTimeString, sizeof(dateTimeString), "%Y-%m-%d %H:%M:%S", localTime);

         PteFile << dateTimeString << endl;
         PteFile << "pagetable of process pid: " << pid << endl;

         vector<int> vec = pagetablemap[pid].vpn_to_pfn;

         if (vec.size() == 0 || (mmap[pid]==0 && vmap[pid]==0))
         {
            PteFile << "process not there in memory so no pte" << endl;
            continue;
         }

         for (int i = 0; i < vec.size(); i++)
         {
            PteFile << i << "->" << vec[i] << endl;
         }
         PteFile << endl;
      }

      if (v[0] == "pteall")
      {

         string ptefile = v[1];

         ofstream PteFileall(ptefile + ".txt", std::ios::app);

         time_t currentTime;
         time(&currentTime);
         tm *localTime = std::localtime(&currentTime);
         char dateTimeString[100];
         strftime(dateTimeString, sizeof(dateTimeString), "%Y-%m-%d %H:%M:%S", localTime);

         PteFileall << dateTimeString << endl;

         for (int i = 1; i < pidcount; i++)
         {
            if (mmap[i] == 1 || vmap[i] == 1) // printing even if the address are in virtual memory
            {
               vector<int> vec = pagetablemap[i].vpn_to_pfn;

               PteFileall << "pagetable of process pid: " << i << endl;
               for (int ii = 0; ii < vec.size(); ii++)
               {
                  PteFileall << ii << "->" << vec[ii] << endl;
               }
               PteFileall << endl;
            }
         }
      }

      if (v[0] == "exit")
      {
         OutputFile << "exited" << endl;
         return 0;
      }

      if (v[0] == "print")
      {
         int start = stoi(v[1]);
         int end = start + stoi(v[2]);

         if(end > M*1024)
         {
            OutputFile<<"physical memory out of bounds"<<endl;
         }
         else
         {
           for (int i = start; i < end; i++)
           {
            OutputFile << "Value of " << i << ": " << GiveValue[i] << endl;
           }
         }
        
      }

      if (v[0] == "swapout")
      {
         int pid = stoi(v[1]);
         int psize = pagetablemap[pid].process_size;

         if (FreeVirtualMemory >= psize)
         {
            FreeMainMemory += pagetablemap[pid].process_size;

            mmap[pid] = 0;

            vector<int> tt = pagetablemap[pid].vpn_to_pfn;

            for (int j = 0; j < tt.size(); j++)
            {
               for (int jj = 0; jj < VirtualPagesNum; jj++)
               {
                  if (FreeVirtualPages[jj] == 0)
                  {
                     FreeVirtualPages[jj] = 1;
                     for (int k = 0; k < P; k++)
                     {
                        vvalue[jj * P + k] = GiveValue[tt[j] * P + k];
                        GiveValue[tt[j] * P + k] = 0;
                     }
                     FreeMainPages[tt[j]] = 0;
                     tt[j] = jj;
                     break;
                  }
               }
            }

            pagetablemap[pid].vpn_to_pfn = tt;
            FreeVirtualMemory -= pagetablemap[pid].process_size;
            vmap[pid] = 1;

            for (int ii = 0; ii < lru.size(); ii++)
            {
               if (lru[ii] == pid)
               {
                  lru.erase(lru.begin() + ii);
                  break;
               }
            }

            OutputFile << "swapout done for pid: " << pid << endl;
         }
         else
         {
            OutputFile << "swapout notdone: no space in virtual memory" << endl;
         }
      }

      if (v[0] == "swapin") // similar to run case where the process is in virtual memory
      {
         int pid = stoi(v[1]);
         if (mmap[pid] == 1)
         {
            OutputFile << "swapin no need : already in main memory" << endl;
         }
         else if (vmap[pid] == 1)
         {
            int psize = pagetablemap[pid].process_size;

            if (FreeMainMemory >= psize)
            {
               FreeVirtualMemory += psize;

               int count = 0;

               vector<int> tt = pagetablemap[pid].vpn_to_pfn;

               for (int j = 0; j < tt.size(); j++)
               {
                  for (int ii = 0; ii < MainPagesNum; ii++)
                  {
                     if (FreeMainPages[ii] == 0)
                     {
                        FreeMainPages[ii] = 1;

                        for (int k = 0; k < P; k++)
                        {
                           GiveValue[ii * P + k] = vvalue[tt[j] * P + k];
                           vvalue[tt[j] * P + k] = 0;
                        }

                        FreeVirtualPages[tt[j]] = 0;
                        tt[j] = ii;
                        break;
                     }
                  }
               }

               pagetablemap[pid].vpn_to_pfn = tt;
               FreeMainMemory -= psize;
               mmap[pid] = 1;
               vmap[pid] = 0;
               lru.push_back(pid);

               // allocate in main memory
            }

            else
            {
               if (psize > M)
               {
                  OutputFile << "lru failed1" << endl; // very big process can't even fit in main memory
                  continue;
               }
               else
               {

                  int spacealloc = 0;
                  for (int kk = 0; kk < lru.size(); kk++)
                  {
                     spacealloc += pagetablemap[lru[kk]].process_size;
                     if (spacealloc + FreeMainMemory >= psize)
                     {
                        break;
                     }
                  }

                  if (spacealloc > FreeVirtualMemory + psize)
                  {
                     OutputFile << "lru failed2" << endl; // the process removed from main memory cant be fitted in virtual
                     continue;
                  }

                  vmap[pid] = 0;

                  FreeVirtualMemory += psize;

                  int flag = 0;

                  while (FreeMainMemory < psize && lru.size())
                  {

                     int rem_pid = lru[0];

                     if (FreeVirtualMemory >= pagetablemap[rem_pid].process_size)
                     {

                        lru.erase(lru.begin());

                        FreeMainMemory += pagetablemap[rem_pid].process_size;

                        mmap[rem_pid] = 0;

                        vector<int> tt = pagetablemap[rem_pid].vpn_to_pfn;

                        for (int j = 0; j < tt.size(); j++)
                        {
                           for (int jj = 0; jj < VirtualPagesNum; jj++)
                           {
                              if (FreeVirtualPages[jj] == 0)
                              {
                                 FreeVirtualPages[jj] = 1;
                                 for (int k = 0; k < P; k++)
                                 {
                                    vvalue[jj * P + k] = GiveValue[tt[j] * P + k];
                                    GiveValue[tt[j] * P + k] = 0;
                                 }
                                 FreeMainPages[tt[j]] = 0;
                                 tt[j] = jj;
                                 break;
                              }
                           }
                        }

                        pagetablemap[rem_pid].vpn_to_pfn = tt;
                        FreeVirtualMemory -= pagetablemap[rem_pid].process_size;
                        vmap[rem_pid] = 1;
                     }
                     else
                     {
                        flag = 1;
                        break;
                     }
                  }

                  if (flag == 1)
                  {
                     OutputFile << "lru failed3" << endl; // same as failed3 error
                     continue;
                  }

                  vector<int> tt = pagetablemap[pid].vpn_to_pfn;

                  for (int j = 0; j < tt.size(); j++)
                  {
                     for (int ii = 0; ii < MainPagesNum; ii++)
                     {
                        if (FreeMainPages[ii] == 0)
                        {
                           FreeMainPages[ii] = 1;

                           for (int k = 0; k < P; k++)
                           {
                              GiveValue[ii * P + k] = vvalue[tt[j] * P + k];
                              vvalue[tt[j] * P + k] = 0;
                           }

                           FreeVirtualPages[tt[j]] = 0;
                           tt[j] = ii;
                           break;
                        }
                     }
                  }

                  pagetablemap[pid].vpn_to_pfn = tt;
                  FreeMainMemory -= psize;
                  mmap[pid] = 1;
                  lru.push_back(pid);
               }
            }

            OutputFile << "swapin done for pid: " << pid << endl;
         }
         else
         {
            OutputFile << "swapin notdone : process not in any memory" << endl;
         }
      }
   }

   InputFile.close();
}
