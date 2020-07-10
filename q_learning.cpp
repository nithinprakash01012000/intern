#include <bits/stdc++.h>
const double alpha=1;
const double epslon=0.1;
const double gaama=1;
using namespace std;
struct Node
{
    int recieving_data_rate;
    bool terminal=true;
    vector<int> v;
};
double reward_fn(Node N1,Node N2)
{
	return(N2.recieving_data_rate-N1.recieving_data_rate);
}
float random_between_0_1()
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) ;
}
class chumma
{
	public:
		int no_of_states;
		double **reward;
		double **q;
		Node *graph;
		chumma()
		{

		}
		chumma(int n)
		{
			no_of_states=n;
			reward=new double*[n];
			q=new double*[n];
			graph=new Node[n];
			srand(time(0));
			for(int i=0;i<n;i++)
			{
				graph[i].recieving_data_rate=rand()%1000;
				cout<<"recieving_data_rate of node "<<i<<"  "<< graph[i].recieving_data_rate<<endl;
			}
			cout<<endl;
			for(int i=0;i<n;i++)
			{
				reward[i]=new double[n];
				q[i]=new double[n];
			}
		}
		void add_edge(int a,int b)
		{
			graph[a].v.push_back(b);
			graph[a].terminal=false;
		}
		int move(int a)
		{

			int action_choosed;
			int actions=graph[a].v.size();
			if(actions==0)
			{
				return -1;
			}
			else
			{
			float w[actions];
			int max_action;
			double max_q=-999999;
			for(int i=0;i<actions;i++)
			{

				if(max_q<=q[a][graph[a].v[i]])
				{
					max_q=q[a][graph[a].v[i]];
					max_action=graph[a].v[i];
				}
			}
			float rand_0_1=random_between_0_1();
			
			if(rand_0_1<=epslon)
			{
				
				float seg=epslon/actions;
				action_choosed=graph[a].v[rand_0_1/seg];
			}
			else
			{
				
				action_choosed=max_action;
			}
			
			double dynamic_reward;
			dynamic_reward=reward_fn(graph[a],graph[action_choosed])  ;
			int best_action_at_new_state;
			int total_actions_at_new_state=graph[action_choosed].v.size();
			if(total_actions_at_new_state==0)
			{
				q[a][action_choosed]=q[a][action_choosed]+alpha*(dynamic_reward-q[a][action_choosed]);
				cout<<"moved from "<<a<< " to "<<action_choosed<<endl;
				return action_choosed;
			}
			int max_action_at_new_state;
			double max_q1=-999999;
			for(int i=0;i<total_actions_at_new_state;i++)
			{
				
				if(max_q1<=q[action_choosed][graph[action_choosed].v[i]])
				{
					max_q1=q[action_choosed][graph[action_choosed].v[i]];
					max_action_at_new_state=graph[action_choosed].v[i];
				}
			}
			q[a][action_choosed]=q[a][action_choosed]+alpha*(dynamic_reward+gaama*(q[action_choosed][max_action_at_new_state])-q[a][action_choosed]);
			cout<<"moved from "<<a<< " to "<<action_choosed<<endl;
			return action_choosed;
		}
		}
};
int main(int argc, char const *argv[])
{
	int num_of_states=5;
	chumma C(num_of_states);
	C.add_edge(0,1);
	C.add_edge(0,2);
	C.add_edge(2,1);
	C.add_edge(1,2);
	C.add_edge(4,3);
	C.add_edge(1,0);
	C.add_edge(2,0);
	C.add_edge(2,3);
	cout<<"graph considered is"<<endl;
	for(int i=0;i<num_of_states;i++)
	{   
		cout<<"State "<<i<<" has outing edges to ------";
		for(int j=0;j<C.graph[i].v.size();j++)
		{
			cout<<C.graph[i].v[j]<<" ";
		}
		cout<<endl;
	}
	cout<<endl;
	int start_state=2;
	int temp_state=start_state;
	cout<<"Tour started "<<start_state<<endl;
	while(true)
	{
		temp_state=C.move(temp_state);
		if(C.graph[temp_state].v.size()==0)
		{
			cout<<"q_table"<<endl;
			for(int i=0;i<num_of_states ;i++)
			{
				for(int j=0;j<num_of_states ;j++)
				{
					cout<<C.q[i][j]<<" ";
				}
				cout<<endl;
			}
			cout<<"process terminated if u wanna go another tour press 1 else 0"<<endl;
			int temp;
			cin>>temp;
			if(temp!=1)
			{
				break;
			}
			else
			{
				temp_state=start_state;
				cout<<endl<<"Tour started "<<start_state<<endl;
				continue;
			}
			
		}
		int cont;
		cout<<"to continue the tour press 1 else 0"<<endl;
		cin>>cont;
		if(cont!=1)
		{
			break;
		}
		
		
	}

	return 0;
}

