#!/bin/bash
echo "enabling the batguard services..."
systemctl enable batguard.service
systemctl enable batguardsleep.service
systemctl daemon-reload
systemctl start batguard.service
systemctl start batguardsleep.service
echo
echo "batguard services enabled"
echo
