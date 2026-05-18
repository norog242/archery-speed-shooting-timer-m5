#ifndef COMMON_CSS_H
#define COMMON_CSS_H

const char* COMMON_CSS = R"rawliteral(
:root {
  --primary: #0066cc;
  --primary-hover: #0052a3;
  --secondary: #666;
  --danger: #cc0000;
  --success-bg: #d4edda;
  --success-text: #155724;
  --error-bg: #f8d7da;
  --error-text: #721c24;
  --info-bg: #fffbcc;
  --info-text: #666;
  --border-color: #ccc;
  --box-bg: #f0f0f0;
  --radius-sm: 0.3em;
  --radius-md: 0.5em;
}

body {
  font-family: sans-serif;
  max-width: 600px;
  margin: 2em auto;
  padding: 1em;
}

h1 {
  color: #333;
}

.status-box {
  background: var(--box-bg);
  padding: 1em;
  border-radius: var(--radius-md);
  margin-bottom: 1.5em;
}

.status-box strong {
  display: block;
  margin-bottom: 0.5em;
}

.form-group {
  margin-bottom: 1.5em;
}

label {
  display: block;
  margin-bottom: 0.5em;
  font-weight: bold;
}

input[type='text'],
input[type='password'],
input[type='number'] {
  width: 100%;
  padding: 0.8em;
  font-size: 1em;
  border: 1px solid var(--border-color);
  border-radius: var(--radius-sm);
  box-sizing: border-box;
}

input[type='range'] {
  width: 100%;
  padding: 0;
}

button {
  padding: 0.8em 2em;
  font-size: 1.1em;
  border: none;
  border-radius: var(--radius-sm);
  cursor: pointer;
  margin-right: 1em;
  margin-top: 0.5em;
}

.btn-primary {
  background: var(--primary);
  color: white;
}

.btn-primary:hover {
  background: var(--primary-hover);
}

.btn-secondary {
  background: var(--secondary);
  color: white;
}

.btn-danger {
  background: var(--danger);
  color: white;
}

.message {
  padding: 1em;
  margin: 1em 0;
  border-radius: var(--radius-sm);
}

.message.success {
  background: var(--success-bg);
  color: var(--success-text);
}

.message.error {
  background: var(--error-bg);
  color: var(--error-text);
}

.info {
  color: var(--info-text);
  font-size: 0.9em;
  margin-top: 0.5em;
}

.info-box {
  margin-top: 2em;
  padding: 1em;
  border-radius: var(--radius-sm);
}

.nav-btn {
  display: inline-block;
  padding: 0.8em 1.5em;
  background: var(--secondary);
  color: white;
  text-decoration: none;
  border-radius: var(--radius-sm);
  font-size: 1.1em;
  margin-right: 1em;
  margin-bottom: 1em;
}

.divider {
  margin-top: 2em;
  padding-top: 2em;
  border-top: 1px solid var(--border-color);
}

.participants-box {
  background: #f9f9f9;
  padding: 1em;
  border-radius: var(--radius-md);
  margin: 1.5em 0;
}

.participant-item {
  padding: 0.5em;
  border-bottom: 1px solid #ddd;
}

.participant-item:last-child {
  border-bottom: none;
}

.participant-name {
  font-weight: bold;
}

.participant-id {
  color: var(--info-text);
  font-size: 0.9em;
  margin-left: 0.5em;
}

.slider-container {
  display: flex;
  align-items: center;
  gap: 1em;
}

.slider-value {
  font-size: 1.5em;
  font-weight: bold;
  color: var(--primary);
  min-width: 100px;
  text-align: center;
}
)rawliteral";

#endif
