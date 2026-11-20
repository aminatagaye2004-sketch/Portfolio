import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import random
import math

class RoverGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("6-Wheel Rover Control Panel - Individual Motor Control")
        self.root.geometry("1400x900")
        self.root.configure(bg='#2C3E50')
        
        # Serial connection
        self.serial_port = None
        self.running = False
        
        # Test mode
        self.test_mode = tk.BooleanVar(value=True)
        
        # Data variables
        self.distance1 = tk.StringVar(value="---")
        self.distance2 = tk.StringVar(value="---")
        
        # IMU Data
        self.roll = tk.StringVar(value="---")
        self.pitch = tk.StringVar(value="---")
        self.yaw = tk.StringVar(value="---")
        self.ax = tk.StringVar(value="---")
        self.ay = tk.StringVar(value="---")
        self.az = tk.StringVar(value="---")
        self.gx = tk.StringVar(value="---")
        self.gy = tk.StringVar(value="---")
        self.gz = tk.StringVar(value="---")
        self.temp = tk.StringVar(value="---")
        
        # Motor speeds (0-100%)
        self.motor_speeds = {
            'M1': 0, 'M2': 0, 'M3': 0,  # Left Front, Left Middle, Left Rear
            'M4': 0, 'M5': 0, 'M6': 0   # Right Front, Right Middle, Right Rear
        }
        
        # Motor names for display
        self.motor_names = {
            'M1': "Motor 1 (Left Front)",
            'M2': "Motor 2 (Left Middle)",
            'M3': "Motor 3 (Left Rear)",
            'M4': "Motor 4 (Right Front)",
            'M5': "Motor 5 (Right Middle)",
            'M6': "Motor 6 (Right Rear)"
        }
        
        # For IMU visualization
        self.current_roll = 0
        self.current_pitch = 0
        self.current_yaw = 0
        
        self.connection_status = tk.StringVar(value="Disconnected")
        
        # Create widgets
        self.create_widgets()
        
        # Start test mode updater
        self.start_test_mode_updater()
        
        self.add_log("GUI initialized successfully - Individual Motor Control Mode")
        
    def create_widgets(self):
        # Main container
        main_container = tk.Frame(self.root, bg='#2C3E50')
        main_container.pack(fill='both', expand=True, padx=10, pady=10)
        
        # ===== TOP SECTION: Connection =====
        top_frame = tk.Frame(main_container, bg='#2C3E50')
        top_frame.pack(fill='x', pady=(0, 10))
        
        self.create_connection_section(top_frame)
        
        # ===== MIDDLE SECTION: Rover Visual and Sensors =====
        middle_frame = tk.Frame(main_container, bg='#2C3E50')
        middle_frame.pack(fill='both', expand=True, pady=10)
        
        # Left: Rover Visualization with Motor Controls
        self.create_rover_with_controls(middle_frame)
        
        # Right: Sensor Data
        right_sensor_frame = tk.Frame(middle_frame, bg='#2C3E50')
        right_sensor_frame.pack(side='right', fill='both', expand=True, padx=(10, 0))
        
        self.create_distance_section(right_sensor_frame)
        self.create_imu_section(right_sensor_frame)
        
        # ===== BOTTOM SECTION: Log =====
        self.create_log_section(main_container)
        
        # ===== KEYBOARD BINDINGS =====
        self.setup_keyboard_controls()
        
    def create_connection_section(self, parent):
        conn_frame = tk.LabelFrame(parent, text="Connection Settings", 
                                   bg='#34495E', fg='white', 
                                   font=('Arial', 11, 'bold'), 
                                   padx=15, pady=10)
        conn_frame.pack(fill='x', expand=True)
        
        tk.Label(conn_frame, text="Serial Port:", bg='#34495E', fg='white', 
                font=('Arial', 10)).grid(row=0, column=0, padx=5, pady=5, sticky='w')
        
        self.port_combo = ttk.Combobox(conn_frame, width=15, state='readonly')
        self.port_combo.grid(row=0, column=1, padx=5, pady=5)
        self.refresh_ports()
        
        refresh_btn = tk.Button(conn_frame, text="🔄", 
                               command=self.refresh_ports,
                               bg='#3498DB', fg='white', 
                               font=('Arial', 9, 'bold'),
                               relief='raised', width=3)
        refresh_btn.grid(row=0, column=2, padx=5, pady=5)
        
        self.connect_btn = tk.Button(conn_frame, text="Connect", 
                                     command=self.toggle_connection,
                                     bg='#27AE60', fg='white', 
                                     font=('Arial', 10, 'bold'),
                                     relief='raised', padx=20, pady=5)
        self.connect_btn.grid(row=0, column=3, padx=5, pady=5)
        
        status_frame = tk.Frame(conn_frame, bg='#34495E')
        status_frame.grid(row=0, column=4, padx=20, pady=5)
        
        tk.Label(status_frame, text="Status:", bg='#34495E', fg='white', 
                font=('Arial', 10)).pack(side='left', padx=(0, 5))
        
        self.status_canvas = tk.Canvas(status_frame, width=15, height=15, 
                                      bg='#34495E', highlightthickness=0)
        self.status_canvas.pack(side='left', padx=(0, 5))
        self.status_indicator = self.status_canvas.create_oval(2, 2, 13, 13, fill='red')
        
        self.status_label = tk.Label(status_frame, textvariable=self.connection_status,
                                     bg='#34495E', fg='#E74C3C', 
                                     font=('Arial', 10, 'bold'))
        self.status_label.pack(side='left')
        
        test_check = tk.Checkbutton(conn_frame, text="Test Mode", 
                                   variable=self.test_mode,
                                   bg='#34495E', fg='white', 
                                   selectcolor='#2C3E50',
                                   font=('Arial', 9),
                                   command=self.toggle_test_mode)
        test_check.grid(row=0, column=5, padx=20, pady=5)
        
    def create_rover_with_controls(self, parent):
        rover_frame = tk.LabelFrame(parent, text="6-Wheel Rover - Individual Motor Control", 
                                   bg='#34495E', fg='white', 
                                   font=('Arial', 12, 'bold'), 
                                   padx=15, pady=15)
        rover_frame.pack(side='left', fill='both', expand=True)
        
        # Canvas for rover visualization
        self.rover_canvas = tk.Canvas(rover_frame, width=500, height=300, 
                                      bg='#2C3E50', highlightthickness=2,
                                      highlightbackground='#3498DB')
        self.rover_canvas.pack(pady=10)
        
        # Draw rover
        self.draw_rover()
        
        # Motor control sliders
        control_frame = tk.Frame(rover_frame, bg='#34495E')
        control_frame.pack(fill='x', pady=10)
        
        # Left side motors (M1, M2, M3)
        left_frame = tk.LabelFrame(control_frame, text="LEFT SIDE MOTORS", 
                                   bg='#2C3E50', fg='#3498DB', 
                                   font=('Arial', 10, 'bold'))
        left_frame.pack(side='left', fill='both', expand=True, padx=5)
        
        self.create_motor_slider(left_frame, "Motor 1 (Left Front)", 'M1', 0, '#3498DB')
        self.create_motor_slider(left_frame, "Motor 2 (Left Middle)", 'M2', 1, '#27AE60')
        self.create_motor_slider(left_frame, "Motor 3 (Left Rear)", 'M3', 2, '#27AE60')
        
        # Right side motors (M4, M5, M6)
        right_frame = tk.LabelFrame(control_frame, text="RIGHT SIDE MOTORS", 
                                    bg='#2C3E50', fg='#E67E22', 
                                    font=('Arial', 10, 'bold'))
        right_frame.pack(side='right', fill='both', expand=True, padx=5)
        
        self.create_motor_slider(right_frame, "Motor 4 (Right Front)", 'M4', 0, '#E67E22')
        self.create_motor_slider(right_frame, "Motor 5 (Right Middle)", 'M5', 1, '#F39C12')
        self.create_motor_slider(right_frame, "Motor 6 (Right Rear)", 'M6', 2, '#F39C12')
        
        # Quick action buttons at bottom of rover frame
        action_frame = tk.Frame(rover_frame, bg='#34495E')
        action_frame.pack(fill='x', pady=10)
        
        button_config = {'width': 10, 'height': 1, 'font': ('Arial', 9, 'bold'), 'relief': 'raised'}
        
        tk.Button(action_frame, text="All Forward", bg='#27AE60', fg='white',
                 command=self.all_forward, **button_config).pack(side='left', padx=5)
        
        tk.Button(action_frame, text="All Stop", bg='#E74C3C', fg='white',
                 command=self.all_stop, **button_config).pack(side='left', padx=5)
        
        tk.Button(action_frame, text="All Backward", bg='#F39C12', fg='white',
                 command=self.all_backward, **button_config).pack(side='left', padx=5)
        
        tk.Button(action_frame, text="Reset All", bg='#3498DB', fg='white',
                 command=self.reset_all_motors, **button_config).pack(side='left', padx=5)
        
        # Master speed control
        master_frame = tk.Frame(action_frame, bg='#34495E')
        master_frame.pack(side='right', padx=10)
        
        tk.Label(master_frame, text="Master Speed:", bg='#34495E', fg='white', 
                font=('Arial', 9)).pack(side='left')
        
        self.master_speed = tk.Scale(master_frame, from_=0, to=100, orient='horizontal',
                                     bg='#2C3E50', fg='white', length=150,
                                     command=self.update_master_speed)
        self.master_speed.set(50)
        self.master_speed.pack(side='left', padx=5)
        
        self.master_speed_label = tk.Label(master_frame, text="50%", bg='#34495E', fg='#00FF00',
                                          font=('Arial', 9, 'bold'))
        self.master_speed_label.pack(side='left')
        
    def create_motor_slider(self, parent, label, motor_id, row, color):
        """Create a motor control slider"""
        frame = tk.Frame(parent, bg='#2C3E50')
        frame.pack(fill='x', pady=5, padx=10)
        
        # Motor label
        tk.Label(frame, text=f"{label}", bg='#2C3E50', fg=color, 
                font=('Arial', 9, 'bold')).grid(row=0, column=0, sticky='w')
        
        # Speed value display
        speed_var = tk.StringVar(value="0")
        speed_label = tk.Label(frame, textvariable=speed_var, bg='#1C2833', fg='#00FF00',
                              font=('Courier', 10, 'bold'), width=4, relief='sunken')
        speed_label.grid(row=0, column=2, padx=5)
        tk.Label(frame, text="%", bg='#2C3E50', fg='white').grid(row=0, column=3)
        
        # Slider (0-100%)
        slider = tk.Scale(frame, from_=-100, to=100, orient='horizontal',
                         bg='#2C3E50', fg='white', highlightthickness=0,
                         troughcolor='#1C2833', activebackground=color,
                         length=150, command=lambda v, m=motor_id, sv=speed_var: self.update_motor_speed(m, v, sv))
        slider.set(0)
        slider.grid(row=1, column=0, columnspan=4, sticky='ew')
        
        # Store references
        self.motor_sliders[motor_id] = slider
        self.motor_speed_vars[motor_id] = speed_var
        
    def draw_rover(self):
        """Draw the rover with wheels labeled M1-M6"""
        self.rover_canvas.delete("all")
        
        # Rover body
        body_x1, body_y1 = 100, 80
        body_x2, body_y2 = 400, 220
        self.rover_canvas.create_rectangle(body_x1, body_y1, body_x2, body_y2,
                                          fill='#34495E', outline='#3498DB', width=3)
        
        self.rover_canvas.create_text(250, 150, text="6-WHEEL ROVER", 
                                      fill='white', font=('Arial', 14, 'bold'))
        
        # Wheel positions with motor numbers
        wheel_y = 150
        wheel_positions = {
            'M1': (130, wheel_y),   # Left Front
            'M2': (210, wheel_y),   # Left Middle
            'M3': (290, wheel_y),   # Left Rear
            'M4': (370, wheel_y),   # Right Front
            'M5': (290, wheel_y),   # Right Middle
            'M6': (370, wheel_y)    # Right Rear
        }
        
        # Draw wheels with motor numbers
        for motor_id, (x, y) in wheel_positions.items():
            color = '#3498DB' if motor_id in ['M1', 'M2', 'M3'] else '#E67E22'
            
            # Draw wheel
            self.rover_canvas.create_oval(x-12, y-12, x+12, y+12, 
                                         fill=color, outline='white', width=2)
            
            # Add motor number
            self.rover_canvas.create_text(x, y+18, text=motor_id, 
                                         fill='white', font=('Arial', 9, 'bold'))
        
        # Add front indicator arrow
        self.rover_canvas.create_polygon([250, 60, 240, 75, 260, 75], 
                                         fill='#F39C12', outline='white')
        self.rover_canvas.create_text(250, 50, text="FRONT", 
                                      fill='#F39C12', font=('Arial', 9, 'bold'))
        
    def create_distance_section(self, parent):
        dist_frame = tk.LabelFrame(parent, text="Distance Sensors (VL53L0X)", 
                                   bg='#34495E', fg='white', 
                                   font=('Arial', 11, 'bold'), 
                                   padx=15, pady=10)
        dist_frame.pack(fill='x', pady=(0, 10))
        
        # Sensor 1
        sensor1_frame = tk.Frame(dist_frame, bg='#2C3E50', relief='ridge', bd=2)
        sensor1_frame.pack(side='left', fill='both', expand=True, padx=5, pady=5)
        
        tk.Label(sensor1_frame, text="Sensor 1 (Front)", 
                bg='#2C3E50', fg='#3498DB', 
                font=('Arial', 10, 'bold')).pack(pady=5)
        
        distance1_display = tk.Label(sensor1_frame, textvariable=self.distance1, 
                                    bg='#1C2833', fg='#00FF00', 
                                    font=('Courier', 20, 'bold'),
                                    relief='sunken', bd=3, padx=15, pady=8)
        distance1_display.pack(pady=5)
        tk.Label(sensor1_frame, text="mm", bg='#2C3E50', fg='white').pack()
        
        # Sensor 2
        sensor2_frame = tk.Frame(dist_frame, bg='#2C3E50', relief='ridge', bd=2)
        sensor2_frame.pack(side='right', fill='both', expand=True, padx=5, pady=5)
        
        tk.Label(sensor2_frame, text="Sensor 2 (Rear)", 
                bg='#2C3E50', fg='#E67E22', 
                font=('Arial', 10, 'bold')).pack(pady=5)
        
        distance2_display = tk.Label(sensor2_frame, textvariable=self.distance2, 
                                    bg='#1C2833', fg='#00FF00', 
                                    font=('Courier', 20, 'bold'),
                                    relief='sunken', bd=3, padx=15, pady=8)
        distance2_display.pack(pady=5)
        tk.Label(sensor2_frame, text="mm", bg='#2C3E50', fg='white').pack()
        
    def create_imu_section(self, parent):
        imu_frame = tk.LabelFrame(parent, text="IMU Sensor (MPU6050)", 
                                  bg='#34495E', fg='white', 
                                  font=('Arial', 11, 'bold'), 
                                  padx=15, pady=10)
        imu_frame.pack(fill='both', expand=True)
        
        # Create horizontal layout for IMU data and visualizer
        imu_content = tk.Frame(imu_frame, bg='#34495E')
        imu_content.pack(fill='both', expand=True)
        
        # Left side - Numerical data
        data_frame = tk.Frame(imu_content, bg='#34495E')
        data_frame.pack(side='left', fill='both', expand=True, padx=10)
        
        # Orientation (with Yaw)
        orient_frame = tk.LabelFrame(data_frame, text="Orientation (Roll, Pitch, Yaw)", 
                                     bg='#2C3E50', fg='#F39C12', 
                                     font=('Arial', 10, 'bold'))
        orient_frame.pack(fill='x', pady=5)
        
        orient_values = tk.Frame(orient_frame, bg='#2C3E50')
        orient_values.pack(pady=5)
        
        # Roll
        tk.Label(orient_values, text="Roll:", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=0, column=0, padx=10, pady=2, sticky='w')
        tk.Label(orient_values, textvariable=self.roll, bg='#1C2833', fg='#00FF00', 
                font=('Courier', 14, 'bold'), width=8, relief='sunken').grid(row=0, column=1, padx=5, pady=2)
        tk.Label(orient_values, text="°", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=0, column=2, padx=5, pady=2)
        
        # Pitch
        tk.Label(orient_values, text="Pitch:", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=1, column=0, padx=10, pady=2, sticky='w')
        tk.Label(orient_values, textvariable=self.pitch, bg='#1C2833', fg='#00FF00', 
                font=('Courier', 14, 'bold'), width=8, relief='sunken').grid(row=1, column=1, padx=5, pady=2)
        tk.Label(orient_values, text="°", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=1, column=2, padx=5, pady=2)
        
        # Yaw
        tk.Label(orient_values, text="Yaw:", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=2, column=0, padx=10, pady=2, sticky='w')
        tk.Label(orient_values, textvariable=self.yaw, bg='#1C2833', fg='#00FF00', 
                font=('Courier', 14, 'bold'), width=8, relief='sunken').grid(row=2, column=1, padx=5, pady=2)
        tk.Label(orient_values, text="°", bg='#2C3E50', fg='white', 
                font=('Arial', 10)).grid(row=2, column=2, padx=5, pady=2)
        
        # Acceleration
        accel_frame = tk.LabelFrame(data_frame, text="Acceleration (g)", 
                                    bg='#2C3E50', fg='#E67E22', 
                                    font=('Arial', 10, 'bold'))
        accel_frame.pack(fill='x', pady=5)
        
        accel_values = tk.Frame(accel_frame, bg='#2C3E50')
        accel_values.pack(pady=5)
        
        for i, (label, var) in enumerate([("X:", self.ax), ("Y:", self.ay), ("Z:", self.az)]):
            tk.Label(accel_values, text=label, bg='#2C3E50', fg='white').grid(row=i, column=0, padx=10, pady=2, sticky='w')
            tk.Label(accel_values, textvariable=var, bg='#1C2833', fg='#00FF00', 
                    font=('Courier', 10), width=8, relief='sunken').grid(row=i, column=1, padx=5, pady=2)
        
        # Gyroscope
        gyro_frame = tk.LabelFrame(data_frame, text="Gyroscope (°/s)", 
                                   bg='#2C3E50', fg='#3498DB', 
                                   font=('Arial', 10, 'bold'))
        gyro_frame.pack(fill='x', pady=5)
        
        gyro_values = tk.Frame(gyro_frame, bg='#2C3E50')
        gyro_values.pack(pady=5)
        
        for i, (label, var) in enumerate([("X:", self.gx), ("Y:", self.gy), ("Z:", self.gz)]):
            tk.Label(gyro_values, text=label, bg='#2C3E50', fg='white').grid(row=i, column=0, padx=10, pady=2, sticky='w')
            tk.Label(gyro_values, textvariable=var, bg='#1C2833', fg='#00FF00', 
                    font=('Courier', 10), width=8, relief='sunken').grid(row=i, column=1, padx=5, pady=2)
        
        # Temperature
        temp_frame = tk.Frame(data_frame, bg='#34495E')
        temp_frame.pack(fill='x', pady=5)
        tk.Label(temp_frame, text="Temperature:", bg='#34495E', fg='white', 
                font=('Arial', 9)).pack(side='left', padx=5)
        tk.Label(temp_frame, textvariable=self.temp, bg='#1C2833', fg='#00FF00', 
                font=('Courier', 10), width=8, relief='sunken').pack(side='left', padx=5)
        tk.Label(temp_frame, text="°C", bg='#34495E', fg='white').pack(side='left')
        
        # Right side - IMU Visualizer Circle (with Yaw indicator)
        visualizer_frame = tk.Frame(imu_content, bg='#34495E')
        visualizer_frame.pack(side='right', padx=20, pady=10)
        
        # Canvas for IMU visualization (circle with horizon line and yaw)
        self.imu_canvas = tk.Canvas(visualizer_frame, width=200, height=200, 
                                    bg='#1C2833', highlightthickness=2, 
                                    highlightbackground='#3498DB')
        self.imu_canvas.pack()
        
        tk.Label(visualizer_frame, text="IMU Balance Visualizer", bg='#34495E', fg='#F39C12', 
                font=('Arial', 9, 'bold')).pack(pady=5)
        
        # Yaw indicator label
        self.yaw_indicator = tk.Label(visualizer_frame, text="Yaw: ---°", 
                                      bg='#34495E', fg='#00FF00', 
                                      font=('Courier', 10, 'bold'))
        self.yaw_indicator.pack(pady=2)
        
        # Draw initial IMU circle
        self.update_imu_visualization()
        
    def update_imu_visualization(self):
        """Draw the IMU visualization circle with horizon line and yaw indicator"""
        if not hasattr(self, 'imu_canvas'):
            return
            
        self.imu_canvas.delete("all")
        width = 200
        height = 200
        center_x = width // 2
        center_y = height // 2
        radius = 80
        
        # Draw outer circle
        self.imu_canvas.create_oval(center_x - radius, center_y - radius, 
                                    center_x + radius, center_y + radius,
                                    outline='#3498DB', width=3)
        
        # Draw crosshair (center lines)
        self.imu_canvas.create_line(center_x, center_y - radius + 10, 
                                    center_x, center_y + radius - 10, 
                                    fill='#34495E', width=1, dash=(2, 4))
        self.imu_canvas.create_line(center_x - radius + 10, center_y, 
                                    center_x + radius - 10, center_y, 
                                    fill='#34495E', width=1, dash=(2, 4))
        
        # Draw center point
        self.imu_canvas.create_oval(center_x - 4, center_y - 4, 
                                    center_x + 4, center_y + 4,
                                    fill='red', outline='red')
        
        # Draw horizon line based on roll angle
        roll_rad = math.radians(self.current_roll)
        line_length = 120
        
        # Calculate line endpoints
        x1 = center_x - (line_length / 2) * math.cos(roll_rad)
        y1 = center_y - (line_length / 2) * math.sin(roll_rad) + (self.current_pitch * 0.6)
        x2 = center_x + (line_length / 2) * math.cos(roll_rad)
        y2 = center_y + (line_length / 2) * math.sin(roll_rad) + (self.current_pitch * 0.6)
        
        self.imu_canvas.create_line(x1, y1, x2, y2, fill='#00FF00', width=3)
        
        # Draw pitch indicator (small circle)
        pitch_y = center_y + (self.current_pitch * 1.2)
        pitch_y = max(center_y - radius + 15, min(center_y + radius - 15, pitch_y))
        self.imu_canvas.create_oval(center_x - 6, pitch_y - 6, 
                                    center_x + 6, pitch_y + 6,
                                    fill='#F39C12', outline='#F39C12')
        
        # Draw yaw indicator (small arrow around the circle)
        yaw_rad = math.radians(self.current_yaw)
        arrow_radius = radius - 10
        arrow_x = center_x + arrow_radius * math.sin(yaw_rad)
        arrow_y = center_y - arrow_radius * math.cos(yaw_rad)
        self.imu_canvas.create_line(center_x, center_y, arrow_x, arrow_y, 
                                    fill='#E67E22', width=2, arrow='last')
        
        # Add labels
        self.imu_canvas.create_text(center_x, center_y - radius - 5, 
                                   text=f"Roll: {self.current_roll:.1f}°", 
                                   fill='#00FF00', font=('Arial', 8))
        self.imu_canvas.create_text(center_x, center_y + radius + 15, 
                                   text=f"Pitch: {self.current_pitch:.1f}°", 
                                   fill='#00FF00', font=('Arial', 8))
        
        # Update yaw indicator label
        self.yaw_indicator.config(text=f"Yaw: {self.current_yaw:.1f}°")
        
        # Add level indicator text
        if abs(self.current_roll) < 5 and abs(self.current_pitch) < 5:
            self.imu_canvas.create_text(center_x, center_y, 
                                       text="LEVEL", fill='#00FF00', 
                                       font=('Arial', 10, 'bold'))
        
    def create_log_section(self, parent):
        log_frame = tk.LabelFrame(parent, text="Activity Log", 
                                 bg='#34495E', fg='white', 
                                 font=('Arial', 11, 'bold'), 
                                 padx=10, pady=5)
        log_frame.pack(fill='x', pady=(10, 0))
        
        scrollbar = tk.Scrollbar(log_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.log_text = tk.Text(log_frame, height=5, 
                               bg='#1C2833', fg='#00FF00', 
                               font=('Courier', 8),
                               yscrollcommand=scrollbar.set,
                               state='normal')
        self.log_text.pack(side='left', fill='both', expand=True)
        scrollbar.config(command=self.log_text.yview)
        
    def update_motor_speed(self, motor_id, value, speed_var):
        """Update individual motor speed"""
        speed = int(float(value))
        self.motor_speeds[motor_id] = speed
        speed_var.set(str(speed))
        
        # Send command
        self.send_motor_command(motor_id, speed)
        
    def send_motor_command(self, motor_id, speed):
        """Send motor command to Pico"""
        if self.serial_port and self.serial_port.is_open and not self.test_mode.get():
            command = f"M:{motor_id}:{speed}\n"
            self.serial_port.write(command.encode())
        
        if self.test_mode.get():
            self.add_log(f"Motor {motor_id}: {speed}%")
        
    def all_forward(self):
        """Set all motors forward"""
        master = self.master_speed.get()
        for motor_id in self.motor_speeds:
            self.motor_speeds[motor_id] = master
            self.motor_sliders[motor_id].set(master)
            self.motor_speed_vars[motor_id].set(str(master))
            self.send_motor_command(motor_id, master)
        self.add_log(f"All motors forward at {master}%")
        
    def all_backward(self):
        """Set all motors backward"""
        master = self.master_speed.get()
        for motor_id in self.motor_speeds:
            self.motor_speeds[motor_id] = -master
            self.motor_sliders[motor_id].set(-master)
            self.motor_speed_vars[motor_id].set(str(-master))
            self.send_motor_command(motor_id, -master)
        self.add_log(f"All motors backward at {master}%")
        
    def all_stop(self):
        """Stop all motors"""
        for motor_id in self.motor_speeds:
            self.motor_speeds[motor_id] = 0
            self.motor_sliders[motor_id].set(0)
            self.motor_speed_vars[motor_id].set("0")
            self.send_motor_command(motor_id, 0)
        self.add_log("All motors stopped")
        
    def reset_all_motors(self):
        """Reset all motor speeds to zero"""
        self.all_stop()
        self.master_speed.set(50)
        self.master_speed_label.config(text="50%")
        
    def update_master_speed(self, value):
        """Update master speed label"""
        self.master_speed_label.config(text=f"{int(float(value))}%")
        
    def setup_keyboard_controls(self):
        """Setup keyboard shortcuts"""
        self.root.bind('<KeyPress-w>', lambda e: self.all_forward())
        self.root.bind('<KeyPress-s>', lambda e: self.all_backward())
        self.root.bind('<KeyPress-space>', lambda e: self.all_stop())
        self.root.bind('<KeyPress-r>', lambda e: self.reset_all_motors())
        
    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)
            self.add_log(f"Found {len(ports)} port(s)")
            
    def toggle_test_mode(self):
        if self.test_mode.get():
            self.add_log("⚠ Test mode enabled - Using simulated data")
        else:
            self.add_log("Test mode disabled - Ready for hardware connection")
            
    def toggle_connection(self):
        if self.serial_port is None or not self.serial_port.is_open:
            self.connect()
        else:
            self.disconnect()
            
    def connect(self):
        try:
            port = self.port_combo.get()
            if not port and not self.test_mode.get():
                messagebox.showerror("Error", "Please select a serial port")
                return
            
            if not self.test_mode.get():
                self.serial_port = serial.Serial(port, 115200, timeout=0.1)
                time.sleep(2)
                self.add_log(f"✓ Connected to {port}")
            else:
                self.add_log("✓ Test mode connection established")
            
            self.running = True
            self.read_thread = threading.Thread(target=self.read_serial, daemon=True)
            self.read_thread.start()
            
            self.update_connection_status(True)
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect:\n{str(e)}")
            self.add_log(f"✗ Connection failed: {str(e)}")
            
    def disconnect(self):
        self.running = False
        self.all_stop()
        time.sleep(0.1)
        
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.add_log("✓ Disconnected from serial port")
        else:
            self.add_log("✓ Test mode disconnected")
        
        self.update_connection_status(False)
        
    def update_connection_status(self, connected):
        if connected:
            self.connection_status.set("Connected")
            self.status_label.config(fg='#27AE60')
            self.status_canvas.itemconfig(self.status_indicator, fill='#27AE60')
            self.connect_btn.config(text="Disconnect", bg='#E74C3C')
        else:
            self.connection_status.set("Disconnected")
            self.status_label.config(fg='#E74C3C')
            self.status_canvas.itemconfig(self.status_indicator, fill='#E74C3C')
            self.connect_btn.config(text="Connect", bg='#27AE60')
            
    def read_serial(self):
        while self.running:
            try:
                if self.test_mode.get():
                    time.sleep(0.1)
                    continue
                    
                if self.serial_port and self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    if line:
                        self.parse_data(line)
            except Exception as e:
                self.add_log(f"✗ Read error: {str(e)}")
            time.sleep(0.05)
            
    def parse_data(self, line):
        try:
            parts = line.split(',')
            for part in parts:
                if ':' in part:
                    key, value = part.split(':')
                    if key == 'DIST1':
                        self.distance1.set(value)
                    elif key == 'DIST2':
                        self.distance2.set(value)
                    elif key == 'ROLL':
                        self.roll.set(f"{float(value):.1f}")
                        self.current_roll = float(value)
                        self.update_imu_visualization()
                    elif key == 'PITCH':
                        self.pitch.set(f"{float(value):.1f}")
                        self.current_pitch = float(value)
                        self.update_imu_visualization()
                    elif key == 'YAW':
                        self.yaw.set(f"{float(value):.1f}")
                        self.current_yaw = float(value)
                        self.update_imu_visualization()
                    elif key == 'AX':
                        self.ax.set(value)
                    elif key == 'AY':
                        self.ay.set(value)
                    elif key == 'AZ':
                        self.az.set(value)
                    elif key == 'GX':
                        self.gx.set(value)
                    elif key == 'GY':
                        self.gy.set(value)
                    elif key == 'GZ':
                        self.gz.set(value)
                    elif key == 'TEMP':
                        self.temp.set(value)
        except Exception as e:
            self.add_log(f"✗ Parse error: {str(e)}")
            
    def start_test_mode_updater(self):
        def update():
            if self.test_mode.get() and self.running:
                # Simulate distance
                self.distance1.set(str(random.randint(50, 2000)))
                self.distance2.set(str(random.randint(50, 2000)))
                
                # Simulate IMU with smooth movement
                self.current_roll += random.uniform(-3, 3)
                self.current_pitch += random.uniform(-3, 3)
                self.current_yaw += random.uniform(-2, 2)
                self.current_roll = max(-45, min(45, self.current_roll))
                self.current_pitch = max(-45, min(45, self.current_pitch))
                if self.current_yaw >= 360:
                    self.current_yaw -= 360
                elif self.current_yaw < 0:
                    self.current_yaw += 360
                    
                self.roll.set(f"{self.current_roll:.1f}")
                self.pitch.set(f"{self.current_pitch:.1f}")
                self.yaw.set(f"{self.current_yaw:.1f}")
                
                # Update IMU visualizer
                self.update_imu_visualization()
                
                # Simulate accelerometer
                self.ax.set(f"{int(self.current_roll * 20)}")
                self.ay.set(f"{int(self.current_pitch * 20)}")
                self.az.set(f"{900 + random.randint(-50, 50)}")
                
                # Simulate gyroscope
                self.gx.set(f"{int(random.uniform(-50, 50))}")
                self.gy.set(f"{int(random.uniform(-50, 50))}")
                self.gz.set(f"{int(random.uniform(-50, 50))}")
                
                # Simulate temperature
                self.temp.set(f"{25 + random.uniform(-2, 3):.1f}")
            
            self.root.after(200, update)
        
        update()
        
    def add_log(self, message):
        if not hasattr(self, 'log_text'):
            return
            
        timestamp = time.strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"
        
        self.log_text.config(state='normal')
        self.log_text.insert('end', log_entry)
        self.log_text.see('end')
        self.log_text.config(state='disabled')
        
        lines = int(self.log_text.index('end-1c').split('.')[0])
        if lines > 100:
            self.log_text.config(state='normal')
            self.log_text.delete('1.0', '2.0')
            self.log_text.config(state='disabled')
            
    def on_closing(self):
        if messagebox.askokcancel("Quit", "Close Rover Control Panel?"):
            self.disconnect()
            self.root.destroy()

# Initialize motor control dictionaries
RoverGUI.motor_sliders = {}
RoverGUI.motor_speed_vars = {}

if __name__ == "__main__":
    root = tk.Tk()
    app = RoverGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
