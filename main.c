#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include <errno.h>
#include <string.h>

int resolv_remote_host(ssh_session session, char *hostname)
{
    ssh_channel channel;
    int rc;
    char buffer[256];
    char *cmd;
    int nbytes;
    channel = ssh_channel_new(session);
    if (channel == NULL)
        return SSH_ERROR;
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        return rc;
    }
    cmd = (char*) malloc((11 + strlen(hostname) + 1) * sizeof(char));
    sprintf(cmd,"dig +short %s", hostname);
    printf("%s\n", cmd);
    rc = ssh_channel_request_exec(channel, cmd);
    if (rc != SSH_OK)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {

        if (write(1, buffer, nbytes) != (unsigned int) nbytes)
        {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return SSH_ERROR;
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;
}

int main(int argc, char *argv[])
{
    char hostname[100];
    char *user = "omarseemas";
    char *password = "odes";
    char *host = "sdf.org";
    ssh_session my_ssh_session;
    int rc;
    if (argc > 1) {
        printf("Searching %s\n", argv[1]);
        strcpy(hostname, argv[1]);
    } else {
        fprintf(stderr, "Please informe a hostname.\n\tEx: eping google.com\n");
        exit(-1);
    }

    // Open session and set options
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL)
        exit(-1);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, host);
    // Connect to server
    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to %s: %s\n",
                host,ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        exit(-1);
    }

    rc = ssh_userauth_password(my_ssh_session, user, password);
    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        exit(-1);
    }

    resolv_remote_host(my_ssh_session, hostname);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    printf("fim\n");
}
