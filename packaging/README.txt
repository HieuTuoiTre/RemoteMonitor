RemoteMonitor - Windows portable build

1. Run monitor_manager.exe on the manager computer.
2. Log in with the demo account:
   Username: admin
   Password: admin
3. Run monitor_agent.exe on the agent computer.
4. Enter the manager computer's LAN IPv4 address in the agent window.
5. Allow TCP port 45454 through Windows Firewall on the manager computer.

Both computers must be on the same LAN. This MVP uses unencrypted TCP and is
intended for controlled lab/LAN demonstrations only. Do not expose port 45454
to the public Internet.
