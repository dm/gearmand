# Gearman is Participating in Hacktoberfest 2021 !!


**Gearman welcomes contributions from the open-source community!**
## please keep the following values in mind while making your meaningful contributions->
### 1. Everyone is Welcome 
      * Participants in Hacktoberfest come from all over the world and represent thousands of unique skill sets. This program welcomes everyone who's already part of the open source software community, and anyone who is interested in “diving in”.
### 2. Quantity is fun, quality is key
      * Participating in Hacktoberfest leads to personal growth, professional opportunities, and community building. However, it all begins with meaningful contributions to open source software. We are committed to prioritizing quality contributions as outlined in our participation rules.
### 3. Short-term action , long-term impact
      * In the open source software community, we are standing on the shoulders of project maintainers and those who came before us. Your participation has a lasting effect on people and technology long after October.

Low quality contributions are discouraged and may be marked as spam by maintainers. examples of low quality contributions are as follows->
* Pull requests that are automated e.g. scripted opening pull requests to remove whitespace / fix typos / optimize images.
* Pull requests that are disruptive e.g. taking someone else's branch/commits and making a pull request.
* Pull requests that are regarded by a project maintainer as a hindrance vs. helping.
* Something that's clearly an attempt to simply +1 your pull request count for October.
* Last but not least, one pull request to fix a typo is fine, but 5 pull requests to remove a stray whitespace is not.



### Please follow the steps given below to make your first contribution ->
You can obtain the latest source tree for Gearman by issuing the following command:

git clone https://github.com/gearman/gearmand

You will need to have git installed in order for this to work.

If you are building from source control (i.e. git) you should setup your environment via:
./bootstrap.sh autoreconf

This will build the files you will need in order to run "./configure".

If you are working on the server here are some handy environmental variables
you can set so that you can debug make test:

GEARMAN_VALGRIND <- runs the server under valgrind.
GEARMAN_MANUAL_GDB <-- runs the server under a remote version of GDB.
GEARMAN_LOG <-- generates a log file for you with the output of the debug for the server

GDB will pause the server while you do run the GDB command.

Coding Style
------------

Variables during assignment should be like:
a= 12;

When in doubt, use (). It means I clearly know that you meant for an
operation to follow a specific order.

Cast return types void when there is a possibility of failure (don't
bother with printf, use common sense):

(void)some_function(...);

New functions should be named "object_verb_(item_to_act_on)". You
should email the list if you are extending the API.

Use spaces after while, for, do, if, else. Don't around anything else.

If/else bracket style is:
if ()
{
}
else
{
}

Make sure structs have a typedef with a _st suffix, enums have a _t
suffix, and functions have a _fn suffix. For example:

typedef struct gearman_task { ... } gearman_task_st;
typedef enum gearman_return { ... } gearman_return_t;
typedef gearman_return_t (gearman_complete_fn)(gearman_task_st *task);


Happy Hacking,
  -Brian
