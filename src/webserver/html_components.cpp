#include "html_components.h"

String getNavbarHtml() {
  return R"rawliteral(
<nav class="navbar navbar-expand-lg navbar-dark shadow-sm">
  <div class="container-fluid px-3">
    <button id="backButton" class="btn btn-link text-light me-2 d-lg-none" onclick="history.back()" style="font-size:1.3rem; display:none;" title="Back">
      <i class="fas fa-arrow-left"></i>
    </button>
    <a class="navbar-brand fw-bold" href="/"><i class="fas fa-fingerprint me-2"></i>Smart Attendance</a>
    <button class="navbar-toggler border-0" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav" aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarNav">
      <ul class="navbar-nav me-auto mb-2 mb-lg-0">
        <li class="nav-item"><a class="nav-link" href="/"><i class="fas fa-home me-1"></i>Home</a></li>
        <li class="nav-item"><a class="nav-link" href="/a2z"><i class="fas fa-calendar-alt me-1"></i>Attendance</a></li>
        <li class="nav-item"><a class="nav-link" href="/names"><i class="fas fa-users me-1"></i>Students</a></li>
        <li class="nav-item"><a class="nav-link" href="/addnew"><i class="fas fa-user-plus me-1"></i>Add New</a></li>
      </ul>
      <ul class="navbar-nav mb-2 mb-lg-0">
        <li class="nav-item"><a class="nav-link" href="/settings"><i class="fas fa-cog me-1"></i>Settings</a></li>
        <li class="nav-item"><a class="nav-link text-danger" href="/logout"><i class="fas fa-sign-out-alt me-1"></i>Logout</a></li>
      </ul>
    </div>
  </div>
</nav>
<style>
  body {
    /* Remove the padding-top override since we want the navbar at the very top */
    /* padding-top: 0 !important; */
    margin: 0;
  }
  .navbar {
    /* Update navbar styles */
    position: fixed !important;
    top: 0;
    left: 0;
    right: 0;
    width: 100%;
    margin-top: 0; /* Remove negative margin */
    z-index: 1050;
    font-size: 1rem;
    background: rgba(30, 60, 114, 0.7) !important;
    backdrop-filter: blur(16px);
    border-radius: 0 0 20px 20px;
    border: 1px solid rgba(255, 255, 255, 0.1);
    transition: all 0.3s ease;
  }
  
  /* Add padding to the body to prevent content from hiding under navbar */
  body {
    padding-top: 76px !important; /* Force the padding */
  }
  .nav-item .nav-link {
    padding: 0.5rem 0.8rem;
    transition: all 0.3s ease;
    border-radius: 8px;
  }
  .nav-item .nav-link:hover {
    background: rgba(255,255,255,0.15);
    transform: translateY(-2px);
  }
  @media (max-width: 991.98px) {
    .navbar .navbar-nav .nav-link {
      padding-left: 1rem;
      padding-right: 1rem;
      font-size: 1.1rem;
      margin-bottom: 0.25rem;
    }
    .navbar .navbar-brand {
      font-size: 1.1rem;
    }
    .navbar-collapse {
      padding: 15px;
      background: rgba(30, 60, 114, 0.85);
      backdrop-filter: blur(12px);
      border-radius: 15px;
      margin-top: 10px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
  }
  @media (max-width: 575.98px) {
    .navbar {
      border-radius: 0 0 15px 15px;
      font-size: 0.98rem;
      padding: 0.4rem 0.5rem;
    }
    .navbar .navbar-brand {
      font-size: 1rem;
    }
    .navbar .navbar-toggler {
      padding: 0.25rem 0.5rem;
      font-size: 1rem;
    }
    .navbar .navbar-nav .nav-link {
      font-size: 1rem;
      padding: 0.6rem 0.8rem;
    }
    .btn-link {
      font-size: 1rem !important;
    }
  }
</style>

<script>
  // This script immediately applies page backgrounds for faster visual feedback
  (function() {
    var path = window.location.pathname;
    var page = 'default';
    
    // Show back button only if not on home page
    var backButton = document.getElementById('backButton');
    if (path !== '/' && path !== '/index.html') {
      if (backButton) backButton.style.display = 'block';
    }
    
    if (path === '/login') {
      page = 'login';
    } else if (path === '/a2z' || path === '/scan' || path.includes('attendance')) {
      page = 'attendance';
    } else if (path === '/names' || path === '/addnew') {
      page = 'students';
    } else if (path === '/settings') {
      page = 'settings';
    }
    
    document.body.setAttribute('data-page', page);
    console.log('Navbar applied background for page:', page);
  })();

  // Add navbar auto-close functionality
  document.addEventListener('click', function(event) {
    const navbar = document.getElementById('navbarNav');
    const navbarToggler = document.querySelector('.navbar-toggler');
    
    // Check if navbar is expanded and click is outside navbar
    if (navbar.classList.contains('show') && 
        !navbar.contains(event.target) && 
        !navbarToggler.contains(event.target)) {
      // Create and dispatch bootstrap collapse event
      const bsCollapse = new bootstrap.Collapse(navbar);
      bsCollapse.hide();
    }
  });
</script>

<!-- Include Bootstrap Bundle with Popper for navbar functionality -->
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>

)rawliteral";
}

String getGlassmorphismStyles() {
  return R"rawliteral(
<style>
  :root {
    --primary-color: #2c3e50;
    --secondary-color: #3498db;
    --accent-color: #e74c3c;
    --success-color: #2ecc71;
    --warning-color: #f1c40f;
    --light-color: #ecf0f1;
    --dark-color: #2c3e50;
    --glass-bg: rgba(255, 255, 255, 0.25);
    --glass-border: rgba(255, 255, 255, 0.25);
    --glass-shadow: 0 8px 32px rgba(31, 38, 135, 0.15);
    
    /* Background gradients for different pages */
    --bg-gradient-default: linear-gradient(135deg, #667eea 0%, #764ba2 50%, #6B8DD6 100%);
    --bg-gradient-login: linear-gradient(135deg, #23243a 0%, #3a3d5c 100%);
    --bg-gradient-attendance: linear-gradient(135deg, #5f72bd 0%, #9b23ea 100%);
    --bg-gradient-students: linear-gradient(135deg, #6a11cb 0%, #2575fc 100%);
    --bg-gradient-settings: linear-gradient(135deg, #3c1053 0%, #ad5389 100%);
  }
  
  html body {
    min-height: 100vh;
    background-attachment: fixed !important;
    background-size: cover !important;
    background-repeat: no-repeat !important;
    background-image: var(--bg-gradient-default) !important;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    position: relative;
    overflow-x: hidden;
  }
  
  /* Apply specific backgrounds based on page URL - use higher specificity */
  html body[data-page="login"] {
    background-image: var(--bg-gradient-login) !important;
  }
  
  html body[data-page="attendance"], 
  html body[data-page="a2z"],
  html body[data-page="scan"] {
    background-image: var(--bg-gradient-attendance) !important;
  }
  
  html body[data-page="names"],
  html body[data-page="addnew"] {
    background-image: var(--bg-gradient-students) !important;
  }
  
  html body[data-page="settings"] {
    background-image: var(--bg-gradient-settings) !important;
  }
  
  /* Animated gradient overlay */
  body::after {
    content: '';
    position: fixed;
    top: 0; left: 0; right: 0; bottom: 0;
    z-index: 0;
    pointer-events: none;
    background: linear-gradient(125deg, rgba(255,255,255,0) 0%, rgba(255,255,255,0.3) 30%, rgba(255,255,255,0) 60%);
    background-size: 200% 200%;
    animation: gradientMovement 8s ease infinite;
  }
  
  @keyframes gradientMovement {
    0% {background-position: 0% 50%}
    50% {background-position: 100% 50%}
    100% {background-position: 0% 50%}
  }
  
  /* SVG pattern overlay for subtle texture */
  body::before {
    content: '';
    position: fixed;
    top: 0; left: 0; right: 0; bottom: 0;
    z-index: 0;
    pointer-events: none;
    opacity: 0.1;
    background: url('data:image/svg+xml;utf8,<svg width="100" height="100" viewBox="0 0 100 100" fill="none" xmlns="http://www.w3.org/2000/svg"><circle cx="50" cy="50" r="48" stroke="%23ffffff" stroke-width="0.5" fill="none"/><circle cx="50" cy="50" r="30" stroke="%23ffffff" stroke-width="0.5" fill="none"/><circle cx="50" cy="50" r="10" stroke="%23ffffff" stroke-width="0.5" fill="none"/></svg>');
    background-size: 150px 150px;
    background-repeat: repeat;
  }
  
  .container {
    position: relative;
    z-index: 1;
    max-width: 1400px;
    padding: 20px;
  }
  
  /* Glassmorphism card */
  .glass-card {
    background: rgba(255, 255, 255, 0.15);
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    border-radius: 24px;
    border: 1px solid rgba(255, 255, 255, 0.18);
    box-shadow: 0 8px 32px rgba(31, 38, 135, 0.1);
    padding: 25px;
    transition: all 0.3s ease;
    margin-bottom: 20px;
    overflow: hidden;
    position: relative;
  }
  
  .glass-card::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    height: 1px;
    background: linear-gradient(90deg, rgba(255,255,255,0), rgba(255,255,255,0.8), rgba(255,255,255,0));
  }
  
  .glass-card:hover {
    transform: translateY(-5px);
    box-shadow: 0 15px 35px rgba(31, 38, 135, 0.2);
    border: 1px solid rgba(255, 255, 255, 0.3);
  }
  
  /* For forms */
  .form-control {
    background: rgba(255, 255, 255, 0.2);
    border: 1px solid rgba(255, 255, 255, 0.2);
    border-radius: 12px;
    padding: 10px 15px;
    transition: all 0.3s ease;
    color: #333;
  }
  
  .form-control:focus {
    background: rgba(255, 255, 255, 0.25);
    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.15);
    border-color: rgba(255, 255, 255, 0.4);
  }
  
  .form-control::placeholder {
    color: rgba(0, 0, 0, 0.5);
  }
  
  /* Buttons */
  .btn-glass {
    background: rgba(255, 255, 255, 0.2);
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    border: 1px solid rgba(255, 255, 255, 0.2);
    border-radius: 12px;
    color: white;
    padding: 10px 20px;
    transition: all 0.3s ease;
    position: relative;
    overflow: hidden;
  }
  
  .btn-glass::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: linear-gradient(120deg, rgba(255,255,255,0) 0%, rgba(255,255,255,0.1) 50%, rgba(255,255,255,0) 100%);
    transform: translateX(-100%);
    transition: all 0.6s ease;
  }
  
  .btn-glass:hover {
    background: rgba(255, 255, 255, 0.25);
    transform: translateY(-3px);
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
  }
  
  .btn-glass:hover::before {
    transform: translateX(100%);
  }
  
  .btn-glass-primary {
    background: rgba(44, 62, 80, 0.4);
    color: white;
  }
  
  .btn-glass-primary:hover {
    background: rgba(44, 62, 80, 0.5);
    color: white;
  }
  
  .btn-glass-success {
    background: rgba(46, 204, 113, 0.4);
    color: white;
  }
  
  .btn-glass-success:hover {
    background: rgba(46, 204, 113, 0.5);
    color: white;
  }
  
  .btn-glass-danger {
    background: rgba(231, 76, 60, 0.4);
    color: white;
  }
  
  .btn-glass-danger:hover {
    background: rgba(231, 76, 60, 0.5);
    color: white;
  }
  
  .btn-glass-warning {
    background: rgba(241, 196, 15, 0.4);
    color: white;
  }
  
  .btn-glass-warning:hover {
    background: rgba(241, 196, 15, 0.5);
    color: white;
  }
  
  /* Tables */
  .table {
    border-collapse: separate;
    border-spacing: 0;
    width: 100%;
  }
  
  .glass-table {
    background: rgba(255, 255, 255, 0.1);
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    border-radius: 16px;
    overflow: hidden;
    border: 1px solid rgba(255, 255, 255, 0.18);
    box-shadow: 0 8px 32px rgba(31, 38, 135, 0.1);
  }
  
  .glass-table thead th {
    background: rgba(44, 62, 80, 0.5);
    color: white;
    padding: 15px;
    font-weight: 500;
    text-transform: uppercase;
    font-size: 0.85rem;
    letter-spacing: 0.5px;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  }
  
  .glass-table tbody tr {
    transition: all 0.3s ease;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  }
  
  .glass-table tbody tr:last-child {
    border-bottom: none;
  }
  
  .glass-table tbody tr:hover {
    background: rgba(255, 255, 255, 0.05);
  }
  
  .glass-table td {
    padding: 12px 15px;
    color: rgba(255, 255, 255, 0.9);
    vertical-align: middle;
  }
  
  /* Card header */
  .glass-card-header {
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    margin-bottom: 20px;
    padding-bottom: 15px;
    font-weight: 600;
    color: rgba(255, 255, 255, 0.9);
  }
  
  /* Badge */
  .glass-badge {
    background: rgba(255, 255, 255, 0.15);
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    border-radius: 50px;
    padding: 5px 12px;
    font-size: 0.8rem;
    font-weight: 500;
    border: 1px solid rgba(255, 255, 255, 0.18);
    color: white;
  }
  
  /* Status indicator */
  .status-indicator {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    display: inline-block;
    margin-right: 5px;
    position: relative;
  }
  
  .status-indicator::after {
    content: '';
    position: absolute;
    top: -2px;
    left: -2px;
    right: -2px;
    bottom: -2px;
    border-radius: 50%;
    background: inherit;
    opacity: 0.4;
    filter: blur(2px);
    z-index: -1;
  }
  
  .status-active {
    background-color: var(--success-color);
    box-shadow: 0 0 8px rgba(46, 204, 113, 0.6);
  }
  
  .status-inactive {
    background-color: var(--accent-color);
    box-shadow: 0 0 8px rgba(231, 76, 60, 0.6);
  }
  
  /* Text colors */
  .text-glass {
    color: rgba(255, 255, 255, 0.9);
  }
  
  .text-glass-muted {
    color: rgba(255, 255, 255, 0.6);
  }
  
  /* Responsive adjustments */
  @media (max-width: 768px) {
    .glass-card {
      padding: 15px;
    }
    
    .container {
      padding: 10px;
    }
  }
</style>

<script>
  // JavaScript to add page-specific data attribute to body
  window.addEventListener('DOMContentLoaded', function() {
    var path = window.location.pathname;
    var page = 'default';
    
    if (path === '/login') {
      page = 'login';
    } else if (path === '/a2z' || path === '/scan' || path.includes('attendance')) {
      page = 'attendance';
    } else if (path === '/names' || path === '/addnew') {
      page = 'students';
    } else if (path === '/settings') {
      page = 'settings';
    }
    
    document.body.setAttribute('data-page', page);
    console.log('Applied page-specific background:', page);
  });
</script>
  )rawliteral";
} 