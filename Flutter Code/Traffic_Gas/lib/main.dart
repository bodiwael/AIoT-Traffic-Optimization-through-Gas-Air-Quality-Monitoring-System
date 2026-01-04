import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(const TrafficMonitorApp());
}

class TrafficMonitorApp extends StatelessWidget {
  const TrafficMonitorApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Traffic Monitor',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        brightness: Brightness.dark,
      ),
      home: const TrafficDashboard(),
    );
  }
}

class TrafficDashboard extends StatefulWidget {
  const TrafficDashboard({Key? key}) : super(key: key);

  @override
  State<TrafficDashboard> createState() => _TrafficDashboardState();
}

class _TrafficDashboardState extends State<TrafficDashboard> {
  final DatabaseReference _database = FirebaseDatabase.instance.ref();
  Map<String, dynamic> traffic1Data = {};
  Map<String, dynamic> traffic2Data = {};

  @override
  void initState() {
    super.initState();
    _setupListeners();
  }

  void _setupListeners() {
    // Listen to traffic1/current
    _database.child('traffic1/current').onValue.listen((event) {
      if (event.snapshot.value != null) {
        setState(() {
          traffic1Data = Map<String, dynamic>.from(event.snapshot.value as Map);
        });
      }
    });

    // Listen to traffic2/current
    _database.child('traffic2/current').onValue.listen((event) {
      if (event.snapshot.value != null) {
        setState(() {
          traffic2Data = Map<String, dynamic>.from(event.snapshot.value as Map);
        });
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Traffic Monitor'),
        centerTitle: true,
      ),
      body: RefreshIndicator(
        onRefresh: () async {
          // Force refresh
          setState(() {});
        },
        child: ListView(
          padding: const EdgeInsets.all(16),
          children: [
            TrafficCard(
              title: 'Traffic 1',
              data: traffic1Data,
              icon: Icons.traffic,
            ),
            const SizedBox(height: 16),
            TrafficCard(
              title: 'Traffic 2',
              data: traffic2Data,
              icon: Icons.traffic,
            ),
          ],
        ),
      ),
    );
  }
}

class TrafficCard extends StatelessWidget {
  final String title;
  final Map<String, dynamic> data;
  final IconData icon;

  const TrafficCard({
    Key? key,
    required this.title,
    required this.data,
    required this.icon,
  }) : super(key: key);

  Color _getTrafficLightColor(String? light) {
    switch (light?.toUpperCase()) {
      case 'RED':
        return Colors.red;
      case 'YELLOW':
        return Colors.yellow;
      case 'GREEN':
        return Colors.green;
      default:
        return Colors.grey;
    }
  }

  Color _getGasLevelColor(String? level) {
    switch (level?.toUpperCase()) {
      case 'LOW':
        return Colors.green;
      case 'MEDIUM':
        return Colors.orange;
      case 'HIGH':
        return Colors.red;
      default:
        return Colors.grey;
    }
  }

  @override
  Widget build(BuildContext context) {
    final gasLevel = data['gas_level'] ?? 'N/A';
    final mq135 = data['mq135'] ?? 0;
    final mq2Calibrated = data['mq2_calibrated'] ?? 0;
    final mq2Raw = data['mq2_raw'] ?? 0;
    final timestamp = data['timestamp'] ?? 0;
    final trafficLight = data['traffic_light'] ?? 'N/A';

    return Card(
      elevation: 4,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      child: ExpansionTile(
        leading: Icon(icon, size: 40, color: _getTrafficLightColor(trafficLight)),
        title: Text(
          title,
          style: const TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
        ),
        subtitle: Text('Light: $trafficLight'),
        children: [
          Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              children: [
                _buildInfoRow(
                  'Traffic Light',
                  trafficLight,
                  Icons.light_mode,
                  _getTrafficLightColor(trafficLight),
                ),
                const Divider(),
                _buildInfoRow(
                  'Gas Level',
                  gasLevel,
                  Icons.cloud,
                  _getGasLevelColor(gasLevel),
                ),
                const Divider(),
                _buildInfoRow(
                  'MQ135',
                  mq135.toString(),
                  Icons.sensors,
                  Colors.blue,
                ),
                const Divider(),
                _buildInfoRow(
                  'MQ2 Calibrated',
                  mq2Calibrated.toString(),
                  Icons.tune,
                  Colors.purple,
                ),
                const Divider(),
                _buildInfoRow(
                  'MQ2 Raw',
                  mq2Raw.toString(),
                  Icons.memory,
                  Colors.teal,
                ),
                const Divider(),
                _buildInfoRow(
                  'Timestamp',
                  timestamp.toString(),
                  Icons.access_time,
                  Colors.grey,
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildInfoRow(String label, String value, IconData icon, Color color) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8),
      child: Row(
        children: [
          Icon(icon, color: color, size: 24),
          const SizedBox(width: 12),
          Expanded(
            child: Text(
              label,
              style: const TextStyle(fontSize: 16, fontWeight: FontWeight.w500),
            ),
          ),
          Text(
            value,
            style: TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.bold,
              color: color,
            ),
          ),
        ],
      ),
    );
  }
}